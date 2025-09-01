from PySide6.QtCore import QObject, QThread, Signal
from PySide6.QtWidgets import QMessageBox, QProgressDialog


class Worker(QObject):
    finished = Signal()
    error = Signal(str)

    def __init__(self, rc, argv, parent=None):
        super().__init__(parent)
        self.rc = rc
        self.argv = argv

    def run(self):
        try:
            self.rc(*self.argv)
            self.finished.emit()
        except AssertionError as ex:
            s = [ln for ln in str(ex).splitlines() if ln.startswith("Error")]
            self.error.emit("\n".join(s))


def show(parent, title, message, callback, argv=[]):
    def on_error(s):
        QMessageBox.critical(parent, "Error", s)

    dialog = QProgressDialog(message, None, 0, 0, parent)
    dialog.setWindowTitle(title)
    dialog.setModal(True)
    dialog.show()

    thread = QThread()
    worker = Worker(callback, argv)
    worker.moveToThread(thread)

    thread.started.connect(worker.run)
    worker.finished.connect(thread.quit)
    worker.finished.connect(dialog.accept)
    worker.error.connect(on_error)
    worker.error.connect(thread.quit)
    worker.error.connect(dialog.close)
    thread.finished.connect(thread.deleteLater)

    thread.start()
    return dialog, thread, worker
