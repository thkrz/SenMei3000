import serial
import time
import sys


class COM:
    def __init__(self, port):
        try:
            self.com = serial.Serial(port)
        except serial.SerialException:
            sys.exit(1)
        self.com.baudrate = 19200
        self.com.bytesize = 8
        self.com.parity = "N"
        self.com.stopbits = 1

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
