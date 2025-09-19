import prompt_toolkit as ptk
import serial
import time


class COM:
    def __init__(self, port):
        self.com = serial.Serial(port)
        self.com.baudrate = 57600
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


if __name__ == "__main__":
    port = "/dev/ttyACM0"
    timeout = 60.0

    com = COM(port)
    while True:
        try:
            msg = ptk.prompt("RYUJIN> ") or None
        except EOFError:
            break
        if msg:
            com.write(msg)
            com.timeout = timeout
            print(com.read_chunk())
    com.close()
