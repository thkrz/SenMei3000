import serial
import time
import sys



class Control:
    def __init__(self, port = "/dev/ttyACM0"):
        self.com = serial.Serial(port)
        self.com.baudrate = 9600
        self.com.bytesize = 8
        self.com.parity = "N"
        self.com.stopbits = 1

    def close(self):
        self.com.close()

    def write(self, s):
        self.com.write(s.encode())
        time.sleep(1)

    def read_until(self, eol):
        b = self.com.read_until(eol.encode())
        s = b.decode("utf-8")
        assert s.endswith(eol)
        return s[:-len(eol)]


def log(s, end="\n"):
    sys.stderr.write(s + end)
    sys.stderr.flush()


if __name__ == "__main__":
    port = "/dev/ttyACM0"
    if len(sys.argv) > 1:
        port = sys.argv[-1]
    ctrl = Control(port)
    ctrl.write("d")
    s = ctrl.read_until("#DUMP")
    if len(s) == 0:
        log("no data on chip")
    else:
        sys.stdout.write(s + "\r\n")
        sys.stdout.flush()
    time.sleep(1)
    ans = input("Format chip[y/N]?: ") or None
    if ans.lower() == "y":
        ctrl.write("f")
        s = ctrl.read_until("#FORMAT")
        log("Chip formatted")
    ctrl.close()
