import os
import sys
from pathlib import Path
from PySide6.QtCore import Slot
from PySide6.QtGui import QIcon
from PySide6.QtWidgets import (
    QApplication,
    QCheckBox,
    QComboBox,
    QFileDialog,
    QFormLayout,
    QHBoxLayout,
    QLineEdit,
    QMainWindow,
    QMessageBox,
    QPushButton,
    QWidget,
)

import arduino
import progress


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self._choose()

        self.icon = QIcon()
        self.icon.addFile(":/Ryujin.png")
        self.setWindowIcon(self.icon)

        self.setWindowTitle(f"{self.sketch.name.upper()} ({self.sketch})")
        self.ports = arduino.port()
        if len(self.ports) == 0:
            sys.exit(1)


        layout = QFormLayout()
        self.createForm(layout)
        container = QWidget()
        container.setLayout(layout)
        self.setCentralWidget(container)

    def _choose(self):
        path = cache()
        while True:
            self.sketch = QFileDialog.getExistingDirectory(
                self,
                "Select a Folder",
                path,
                QFileDialog.ShowDirsOnly | QFileDialog.DontResolveSymlinks,
            )
            if not self.sketch:
                sys.exit(0)
            path = self.sketch
            self.sketch = Path(self.sketch)
            fwf = self.sketch / "fw.txt"
            if fwf.exists():
                break
            self._error("No firmware version found")
        with open(fwf) as f:
            self.firmware = f.read().strip()
        cache(self.sketch)

    @Slot(str)
    def _error(self, msg):
        QMessageBox.critical(self, "Error", msg)

    def info(self):
        d = {
            "FIRMWARE": f"-> {self.firmware}",
            "STAT_CTRL_ID": "",
            "APN": "iot.1nce.net",
            "MI_MINUTE": "15",
            "LEGACY_BUILT": "0",
        }
        try:
            s = arduino.send(self.port, "i")
            for ln in s.splitlines():
                k, v = tuple([p.strip() for p in ln.split("=")])
                if k == "FIRMWARE" and v != self.firmware:
                    v += " -> " + self.firmware
                d[k] = v
        except (AssertionError, ValueError):
            pass
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
            elif k == "LEGACY_BUILT":
                le = QCheckBox()
                le.setChecked(bool(int(v)))
                self.properties[k] = le
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
        s = arduino.send(self.port, "d")
        if len(s) > 0:
            path, _ = QFileDialog.getSaveFileName(
                self, "Save dump", "", "TextFiles (*.txt);;All Files (*)"
            )
            if path:
                with open(path, "w") as f:
                    f.write(s)

    def chipErase(self):
        def rc(port):
            arduino.send(port, "f", timeout=30.0)

        self.dialog, self.thread, self.worker = progress.show(
            self, "W25Q", "Erasing chip...", self._error, rc, [self.port]
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
            elif k == "LEGACY_BUILT":
                flags[k] = str(int(v.isChecked()))
            else:
                flags[k] = f'"{v.text()}"'

        self.dialog, self.thread, self.worker = progress.show(
            self,
            self.port,
            "Uploading...",
            self._error,
            rc,
            [self.sketch, self.port, flags],
        )


def cache(p=None):
    try:
        wd = Path(os.getenv("XDG_CACHE_HOME"))
    except TypeError:
        wd = Path(os.getenv("HOME")) / ".cache"
    _p = wd / "ryujin_path.txt"
    if p is None:
        if _p.exists():
            with open(_p) as f:
                return f.read()
        return ""
    with open(_p, "w") as f:
        f.write(str(p))


if __name__ == "__main__":
    app = QApplication(sys.argv)
    wnd = MainWindow()
    wnd.show()

    sys.exit(app.exec())
