#!/usr/bin/python3
import serial
import time
import sys


class Control:
    def __init__(self, port):
        try:
            self.com = serial.Serial(port)
        except serial.SerialException:
            log(f"could not open {port}")
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


def log(s, end="\n"):
    sys.stderr.write(s + end)
    sys.stderr.flush()


def usage():
    log("usage: ctrl [port] [-c CMD]")
    sys.exit(1)


if __name__ == "__main__":
    port = "/dev/ttyACM0"
    cmd = "d"
    argv = iter(sys.argv[1:])
    for arg in argv:
        if arg.startswith("-c"):
            if len(arg) > 2:
                cmd = arg[2:]
            else:
                try:
                    cmd = next(argv)
                except StopIteration:
                    usage()
        elif arg == "-?" or arg == "-h":
            usage()
        else:
            port = arg
    with Control(port) as ctrl:
        ctrl.write(cmd)
        s = ctrl.read_chunk()
        sys.stdout.write(s)
        sys.stdout.flush()
    sys.exit(0)
