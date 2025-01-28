import serial
import time

port = "/dev/ttyACM0"


if __name__ == "__main__":
    com = serial.Serial(port)
    com.baudrate = 9600
    com.bytesize = 8
    com.parity = "N"
    com.stopbits = 1
    time.sleep(3)
    com.write("!".encode())
    ln = ""
    while ln != "!":
        ln = com.readline()
        print(ln, end="")
    com.close()
