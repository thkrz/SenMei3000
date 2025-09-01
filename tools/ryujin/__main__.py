import sys
from pathlib import Path
from PySide6.QtCore import QObject, Signal, QThread
from PySide6.QtWidgets import (
    QApplication,
    QMainWindow,
    QMessageBox,
    QProgressDialog,
    QFileDialog,
    QFormLayout,
    QWidget,
    QPushButton,
    QComboBox,
    QLineEdit,
    QHBoxLayout,
)

# from . import arduino
import arduino


class Erase(QObject):
    finished = Signal()

    def __init__(self, port, parent=None):
        super().__init__(parent)
        self.port = port

    def run(self):
        try:
            with arduino.COM(self.port) as com:
                com.write("f")
                com.read_chunk()
            self.finished.emit()
        except AssertionError:
            self.error.emit("Error: no connection to " + self.port)


class Upload(QObject):
    finished = Signal()
    error = Signal(str)

    def __init__(self, port, path, flags, parent=None):
        super().__init__(parent)
        self.port = port
        self.path = path
        self.flags = flags

    def run(self):
        try:
            arduino.compile(self.path, self.flags)
            arduino.upload(self.path, self.port)
            self.finished.emit()
        except AssertionError as ex:
            self.error.emit(str(ex))


class MainWindow(QMainWindow):
    def __init__(self, path=None):
        super().__init__()

        if path is None:
            self.path = QFileDialog.getExistingDirectory(
                self,
                "Select a Folder",
                "",
                QFileDialog.ShowDirsOnly | QFileDialog.DontResolveSymlinks,
            )
            if not self.path:
                sys.exit(1)
        else:
            self.path = path
        if not isinstance(self.path, Path):
            self.path = Path(self.path)
        with open(self.path / "fw.txt") as f:
            self.firmware = f.read().strip()

        self.setWindowTitle(f"RYUJIN - {self.path}")
        self.ports = arduino.port()
        if len(self.ports) == 0:
            sys.exit(1)

        layout = QFormLayout()
        self.createForm(layout)
        container = QWidget()
        container.setLayout(layout)
        self.setCentralWidget(container)

    def info(self):
        d = {}
        with arduino.COM(self.port) as com:
            com.write("i")
            s = com.read_chunk()
            for ln in s.splitlines():
                k, v = tuple([p.strip() for p in ln.split(":")])
                if k == "FIRMWARE" and v != self.firmware:
                    v += " -> " + self.firmware
                d[k] = v
        return d

    def createForm(self, form):
        combobox = QComboBox()
        combobox.currentIndexChanged.connect(self.selectPort)
        combobox.addItems(self.ports)
        form.addRow("Port:", combobox)

        self.properties = {}
        for k, v in self.info().items():
            le = QLineEdit()
            le.setText(v)
            self.properties[k] = le
            if k == "FIRMWARE":
                le.setReadOnly(True)
            elif k.startswith("MI"):
                k = "MI"
                lo = QHBoxLayout()
                self.unit = QComboBox()
                self.unit.addItems(["MINUTE", "HOUR"])
                lo.addWidget(self.unit)
                lo.addWidget(le)
                le = lo
            form.addRow(k, le)

        fwupd = QPushButton("Upload firmware")
        fwupd.clicked.connect(self.upload)
        form.addRow(fwupd)

        layout = QHBoxLayout()
        btn = QPushButton("Erase chip")
        btn.clicked.connect(self.chipErase)
        layout.addWidget(btn)
        btn = QPushButton("Dump chip")
        btn.clicked.connect(self.chipDump)
        layout.addWidget(btn)

        form.addRow("Memory:", layout)

    def chipDump(self):
        with arduino.COM(self.port) as com:
            com.write("d")
            s = com.read_chunk()
        if len(s) > 0:
            path, _ = QFileDialog.getSaveFileName(
                self, "Save dump", "", "TextFiles (*.txt);;All Files (*)"
            )
            if path:
                with open(path, "w") as f:
                    f.write(s)

    def chipErase(self):
        self.progress_dialog = QProgressDialog("Erasing chip...", None, 0, 0, self)
        self.progress_dialog.setWindowTitle("W25Q")
        self.progress_dialog.setModal(True)
        self.progress_dialog.show()

        self.thread = QThread()
        self.worker = Erase(self.port)
        self.worker.moveToThread(self.thread)

        self.thread.started.connect(self.worker.run)
        self.worker.finished.connect(self.thread.quit)
        self.worker.finished.connect(self.progress_dialog.accept)
        self.thread.finished.connect(self.thread.deleteLater)

        self.thread.start()

    def selectPort(self, index):
        self.port = self.ports[index]

    def upload(self):
        flags = {}
        for k, v in self.properties.items():
            if k == "FIRMWARE":
                flags[k] = f'"{self.firmware}"'
            elif k.startswith("MI"):
                k = f"MI_{self.unit.currentText()}"
                flags[k] = v.text()
            else:
                flags[k] = f'"{v.text()}"'

        self.progress_dialog = QProgressDialog("Uploading...", None, 0, 0, self)
        self.progress_dialog.setWindowTitle(self.port)
        self.progress_dialog.setModal(True)
        self.progress_dialog.show()

        self.thread = QThread()
        self.worker = Upload(self.port, self.path, flags)
        self.worker.moveToThread(self.thread)

        self.thread.started.connect(self.worker.run)
        self.worker.finished.connect(self.thread.quit)
        self.worker.finished.connect(self.progress_dialog.accept)
        self.worker.error.connect(self.thread.quit)
        self.worker.error.connect(self.onError)
        self.thread.finished.connect(self.thread.deleteLater)

        self.thread.start()

    def onError(self, errmsg):
        self.progress_dialog.close()
        msg = "\n".join([ln for ln in errmsg.splitlines() if ln.startswith("Error")])
        QMessageBox.critical(self, "Error", msg)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    wnd = MainWindow()
    wnd.show()

    app.exec()
