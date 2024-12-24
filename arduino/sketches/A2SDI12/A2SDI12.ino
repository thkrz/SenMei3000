#include <EEPROM.h>
#include <SDI12.h>

#define CMD_LEN 32
#define EE_ADDR 0
#define NUM_CON 6

struct Connector {
  char addr;
  bool active;
  float odd, even;
};

Connector conn[NUM_CON];
SDI12 socket(1);
char buf[CMD_LEN]
int len = 0;
int pin[] = {
  A0, A1, A2, A3, A4, A5,
  A6, A7, A8, A9, A10, A11
};

int index(char a) {
  for (int i = 0; i < NUM_SEN; i++)
    if (conn[i].active && conn[i].addr == a)
      return i;
  return -1;
}

void measure(int i) {
  int k = i * 2;
  for (int j = 0; j < 3; j++) {
    conn[i].even += analogRead(pin[k]);
    conn[i].odd += analogRead(pin[k+1]);
  }
  conn[i].even /= 3.0;
  conn[i].odd /= 3.0;
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
      r = "";
      break;
    case 'M':
      m = true;
      r = "0012";
      break;
    case 'D':
      r = data(i);
      break;
    case 'A':
      addr = buf[2];
      conn[i].addr = addr;
      EEPROM.write(EE_ADDR+i, addr);
      break;
    }

  String s = String(addr) + r + "\r\n";
  socket.sendResponse(s);
  if (m)
    measure(i);
}

void setup() {
  for (int i = 0; i < NUM_CON; i++) {
    int j = i * 2;
    pinMode(pin[j], INPUT_PULLUP);
    conn[i].active = analogRead(pin[j]) > 0;
    pinMode(pin[j], INPUT);
    if (active)
      conn[i].addr = EEPROM.read(EE_ADDR+i);
  }
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
