// https://github.com/EnviroDIY/Arduino-SDI-12/blob/master/examples/h_SDI-12_slave_implementation/h_SDI-12_slave_implementation.ino
#include <SDI12.h>

#define CMD_LEN 32
#define NUM_SEN 6

struct {
  char addr;
  int pin;
  float f(int);
} Sensor;

SDI12 socket(1);
char buf[CMD_LEN]
int len = 0;
char msr = 'A';

void readSamples() {
}

bool rc() {
  char addr = buf[0];
  if (addr < msr || addr > msr + NUM_SEN)
    return false;
  String r = "";
  bool state = false;
  if (len > 1)
    switch (buf[1]) {
    case 'I':
      break;
    case 'M':
      state = true;
      break;
    case 'D':
      break;
    case 'A':
      if (addr == msr) {
        addr = buf[2];
        msr = addr;
      }
      break;
    }

  String s = String(addr) + r + "\r\n";
  socket.sendResponse(s);
  return state;
}

void setup() {
  socket.begin();
  delay(500);
  socket.forceListen();
}

void loop() {
  if (socket.available()) {
    char c = socket.read();
    if (c == '!') {
      socket.clearBuffer();
      socket.forceHold();
      if(cmd())
        readSamples();
      len = 0;
      socket.forceListen();
    } else if (len < CMD_LEN)
      buf[len++] = c;
  }
}
