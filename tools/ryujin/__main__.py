import os
import sys
from pathlib import Path
from PySide6.QtWidgets import (
    QApplication,
    QMainWindow,
    QFileDialog,
    QFormLayout,
    QWidget,
    QPushButton,
    QComboBox,
    QLineEdit,
    QHBoxLayout,
    QMessageBox,
)

import arduino
import progress


class MainWindow(QMainWindow):
    def __init__(self, path=""):
        super().__init__()

        self.path = QFileDialog.getExistingDirectory(
            self,
            "Select a Folder",
            path,
            QFileDialog.ShowDirsOnly | QFileDialog.DontResolveSymlinks,
        )
        if not self.path:
            sys.exit(1)
        sketch(self.path)

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
            form.addRow(k + ":", le)

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
        def rc(port):
            with arduino.COM(port) as com:
                com.write("f")
                com.read_chunk()

        self.dialog, self.thread, self.worker = progress.show(
            self, "W25Q", "Erasing chip...", rc, [self.port]
        )

    def selectPort(self, index):
        self.port = self.ports[index]

    def upload(self):
        def rc(path, port, flags):
            arduino.compile(path, flags)
            arduino.upload(path, port)

        flags = {}
        for k, v in self.properties.items():
            if k == "FIRMWARE":
                flags[k] = f'"{self.firmware}"'
            elif k.startswith("MI"):
                k = f"MI_{self.unit.currentText()}"
                flags[k] = v.text()
            else:
                flags[k] = f'"{v.text()}"'

        self.dialog, self.thread, self.worker = progress.show(
            self, self.port, "Uploading...", rc, [self.path, self.port, flags]
        )


def sketch(p=None):
    cache = os.getenv("XDG_CACHE_HOME")
    if not cache:
        cache = Path(os.getenv("HOME")) / ".cache"
    else:
        cache = Path(cache)
    _path = cache / "ryujin_path.txt"
    if p is None:
        if _path.exists():
            with open(_path) as f:
                return f.read()
        return ""
    with open(_path, "w") as f:
        f.write(p)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    wnd = MainWindow(sketch())
    wnd.show()

    app.exec()
