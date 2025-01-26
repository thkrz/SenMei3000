import serial
import time

port = "/dev/ttyACM0"
com = serial.Serial(port)


def loop():
    cmd = input("RYUJIN> ")
    com.write(cmd.encode())
    s = com.readline()
    print(s)


if __name__ == "__main__":
    com.baudrate = 9600
    com.bytesize = 8
    com.parity = "N"
    com.stopbits = 1
    time.sleep(3)

    try:
        while True:
            loop()
    except EOFError:
        pass
    except KeyboardInterrupt:
        pass
