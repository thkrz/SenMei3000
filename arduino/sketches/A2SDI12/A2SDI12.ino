// https://github.com/EnviroDIY/Arduino-SDI-12/blob/master/examples/h_SDI-12_slave_implementation/h_SDI-12_slave_implementation.ino
#include <EEPROM.h>
#include <SDI12.h>

#define CMD_LEN 32
#define EE_ADDR 0
#define NUM_SEN 6

SDI12 socket(1);
Sensor sen[NUM_SEN];
char tr[NUM_SEN];
char buf[CMD_LEN]
int len = 0;

void rc() {
  char addr = buf[0];
  bool valid = false;
  for (int j = 0; j < NUM_SEN && !valid; j++)
    valid = tr[j] == addr;
  if (!valid)
    return;
  String r = "";
  bool m = false;
  if (len > 1)
    switch (buf[1]) {
    case 'I':
      r = sen[i].ident();
      break;
    case 'M':
      m = true;
      r = sen[i].measure();
      break;
    case 'D':
      r = sen[i].data();
      break;
    case 'A':
      addr = buf[2];
      msr = addr;
      break;
    }

  String s = String(addr) + r + "\r\n";
  socket.sendResponse(s);
  if (m)
    sen[i].readSample();
}

void setup() {
  for (int i = 0; i < NUM_SEN; i++)
    tr[i] = EEPROM.read(EE_ADDR+i);
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
      rc();
      len = 0;
      socket.forceListen();
    } else if (len < CMD_LEN)
      buf[len++] = c;
  }
}
