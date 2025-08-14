import prompt_toolkit as ptk
import serial
import time
import sys

if __name__ == "__main__":
    with serial.Serial("/dev/ttyACM0") as arduino:
        arduino.baudrate = 19200
        arduino.bytesize = 8
        arduino.parity = "N"
        arduino.stopbits = 1
        time.sleep(3)
        while True:
            try:
                cmd = ptk.prompt("SDI12> ")
            except EOFError:
                sys.exit(0)
            if not cmd.endswith("!"):
                continue
            arduino.write(bytes(cmd, "utf-8"))
            time.sleep(0.05)
            res = arduino.readline()
            sys.stderr.write(res.decode("utf-8"))
            sys.stderr.flush()
