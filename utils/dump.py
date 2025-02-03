import serial
import time
import sys

port = "/dev/ttyACM0"


if __name__ == "__main__":
    if len(sys.argv) > 1:
        port = sys.argv[-1]
    com = serial.Serial(port)
    com.baudrate = 9600
    com.bytesize = 8
    com.parity = "N"
    com.stopbits = 1
    time.sleep(1)
    com.write("!".encode())
    s = com.read_until("#".encode())[:-1]
    com.close()
    sys.stdout.write(s.decode("utf-8"))
    sys.stdout.flush()
