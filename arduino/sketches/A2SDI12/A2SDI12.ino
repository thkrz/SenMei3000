#include <EEPROM.h>
#include <SDI12.h>

#define CMD_LEN 32
#define EE_ADDR 0
#define NUM_SEN 6

SDI12 socket(1);
Sensor sens[NUM_SEN];
char tab[NUM_SEN];
char buf[CMD_LEN]
int len = 0;

int index(char a) {
  for (int i = 0; i < NUM_SEN; i++)
    if (tab[i] == a)
      return i;
  return -1;
}

void rc() {
  char addr = buf[0];
  int i = index(addr);
  if (i < 0)
    return;
  String r = "";
  bool m = false;
  if (len > 1)
    switch (buf[1]) {
    case 'I':
      r = sens[i].ident();
      break;
    case 'M':
      m = true;
      r = sens[i].prepare();
      break;
    case 'D':
      r = sens[i].data(buf[2]);
      break;
    case 'A':
      addr = buf[2];
      tab[i] = addr;
      EEPROM.write(EE_ADDR+i, addr);
      break;
    }

  String s = String(addr) + r + "\r\n";
  socket.sendResponse(s);
  if (m)
    sens[i].measure();
}

void setup() {
  for (int i = 0; i < NUM_SEN; i++)
    tab[i] = EEPROM.read(EE_ADDR+i);
  socket.begin();
  delay(100);
  socket.forceListen();
}

void loop() {
  if (socket.available()) {
    char c = socket.read();
    if (c == '!') {
      socket.clearBuffer();
      socket.forceHold();
      if (len > 0) {
        rc();
        len = 0;
      }
      socket.forceListen();
    } else if (len < CMD_LEN)
      buf[len++] = c;
  }
}
