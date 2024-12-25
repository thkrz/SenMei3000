#include <EEPROM.h>
#include <SDI12.h>

#define CMD_LEN 32
#define EE_ADDR 0
#define NUM_CON 6

struct Connector {
  char addr;
  bool active;
  float u[2];
};

Connector conn[NUM_CON];
SDI12 socket(1);
char buf[CMD_LEN]
int len = 0;
int pin[] = {
  A0, A1, A2, A3, A4, A5,
  A6, A7, A8, A9, A10, A11
};
float map[](float) = {
  smt100moist,
  smt100temp
};

void chkactive(int i) {
  int j = i * 2;
  pinMode(pin[j], INPUT_PULLUP);
  delay(10);
  conn[i].active = analogRead(pin[j]) > 0;
  pinMode(pin[j], INPUT);
} 

String data(int i) {
  Connector *c = &conn[i];
  String s = "";
  for (int j = 0; j < 2; j++) {
    float a = map[j](c->u[j]);
    if (a >= 0)
      s += "+";
    s += String(a);
  }
  return s;
}

int index(char a) {
  for (int i = 0; i < NUM_SEN; i++)
    if (conn[i].addr == a)
      return i;
  return -1;
}

void measure(int i) {
  Connector *c = &conn[i];
  int k = i * 2;
  for (int j = 0; j < 3; j++) {
    c->u[0] += analogRead(pin[k]);
    c->u[1] += analogRead(pin[k+1]);
  }
  c->u[0] *= 0.001629;
  c->u[1] *= 0.001629;
}

void rc() {
  char addr = buf[0];
  int i = index(addr);
  if (i < 0)
    return;
  if (len == 1)
    chkactive();
  if (!conn[i].active)
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
  for (int i = 0; i < NUM_CON; i++)
    conn[i].addr = EEPROM.read(EE_ADDR+i);
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
