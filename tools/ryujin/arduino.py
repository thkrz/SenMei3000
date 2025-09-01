import serial
import time
import subprocess

# FBQN = "arduino:samd:mkrnb1500"
FBQN = "arduino:samd:mkrzero"


class COM:
    def __init__(self, port):
        self.com = serial.Serial(port)
        self.com.baudrate = 19200
        self.com.bytesize = 8
        self.com.parity = "N"
        self.com.stopbits = 1
        # self.com.timeout = 3.0

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()

    def close(self):
        self.com.close()

    def write(self, s):
        self.com.write(s.encode())
        time.sleep(1)

    def read_chunk(self):
        s = self.read_until("#")
        self.com.reset_input_buffer()
        return s

    def read_until(self, eol):
        b = self.com.read_until(eol.encode())
        s = b.decode("utf-8")
        assert s.endswith(eol), "read error"
        return s[: -len(eol)]

    @property
    def timeout(self):
        return self.com.timeout

    @timeout.setter
    def timeout(self, n):
        self.com.timeout = n


def _arduino_cli(argv):
    p = subprocess.Popen(
        ["arduino-cli"] + [str(arg) for arg in argv],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    out, err = p.communicate()
    return p.returncode, out, err


def compile(path, flags={}):
    argv = ["compile", "-b", FBQN]
    if len(flags) > 0:
        flags = [f"-D{k}={v}" for k, v in flags.items()]
        properties = f'compiler.cpp.extra_flags={" ".join(flags)}'
        argv += ["--build-propert", properties]
    argv.append(path)
    r, out, err = _arduino_cli(argv)
    assert r == 0, err


def upload(path, port):
    r, out, err = _arduino_cli(["upload", "-p", port, "-b", FBQN, path])
    assert r == 0, err


def port():
    r, out, _ = _arduino_cli(["board", "list"])
    assert r == 0
    return [s.split()[0] for s in out.splitlines() if FBQN in s]
