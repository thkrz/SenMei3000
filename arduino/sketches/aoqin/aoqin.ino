#include <EEPROM.h>
#include <SDI12.h>

#include "smt100.h"

#define CMD_LEN 8
#define EE_ADDR 0
#define NUM_CON 6
#define BUS_PIN 9

struct Pin {
  int8_t num;
  bool active;
};

class Connector {
  private:
    char addr;
    float u[2];
    Pin pin[2];
};


Term term[NUM_CON];
SDI12 socket(BUS_PIN);
char buf[CMD_LEN];
int len = 0;
int pin[] = {
  A0, A1, A2, A3, A4, A5,
  A6, A7, A8, A9, A10, A11
};

String data(int i) {
  static String s;

  Term *c = &term[i];
  s = "";
  for (int j = 0; j < 2; j++) {
    float a = tr[j](c->u[j]);
    if (a >= 0)
      s += '+';
    s += String(a);
  }
  return s;
}

int index(char a) {
  for (int i = 0; i < NUM_CON; i++)
    if (term[i].addr == a)
      return i;
  return -1;
}

void measure(int i) {
  Term *c = &term[i];
  analogRead(pin[i]);
  analogRead(pin[i+6]);
  for (int j = 0; j < 3; j++) {
    c->u[0] += analogRead(pin[i]);
    c->u[1] += analogRead(pin[i+6]);
  }
  c->u[0] *= 0.001629;
  c->u[1] *= 0.001629;
}

char peekaddr(int a) {
  static const char i = '0';

  char c = EEPROM.read(EE_ADDR + a);
  if (!((c >= '0' && c <= '9') ||
    (c >= 'a' && c <= 'z') ||
    (c >= 'A' && c <= 'Z'))) {
    c = i + a;
    EEPROM.write(EE_ADDR + a, c);
  }
  return c;
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
      r = INFO;
      break;
    case 'M':
      m = true;
      r = WAIT;
      break;
    case 'D':
      r = data(i);
      break;
    case 'A':
      addr = buf[2];
      term[i].addr = addr;
      EEPROM.write(EE_ADDR+i, addr);
      break;
    }

  String s = String(addr) + r + "\r\n";
  socket.sendResponse(s);
  if (m)
    measure(i);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < NUM_CON; i++)
    term[i].addr = peekaddr(i);
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
      if (len > 0) {
        rc();
        len = 0;
      }
      socket.forceListen();
    } else if (c > 0 && len < CMD_LEN)
      buf[len++] = c;
  }
}
