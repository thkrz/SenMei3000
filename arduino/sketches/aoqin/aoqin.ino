#include <EEPROM.h>
#include <SDI12.h>

#include "SMT100.h"

#define CMD_LEN 8
#define EE_ADDR 0
#define NUM_CON 6
#define BUS_PIN 13

Block blk[NUM_CON] = {
  SMT100(7, A0, A1),
  SMT100(5, A2, A3),
  SMT100(4, A4, A5),
  SMT100(3, A6, A7),
  SMT100(2, A8, A9),
  SMT100(1, A10, A11)
};

SDI12 socket(BUS_PIN);
char buf[CMD_LEN];
int len = 0;

int index(char a) {
  for (int i = 0; i < NUM_CON; i++)
    if (blk[i].addr == a)
      return i;
  return -1;
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
  Block *b = &blk[i];
  if (!b->isConnected())
    return;
  String r = "";
  bool rs = false;
  if (len > 1)
    switch (buf[1]) {
    case 'I':
      r = b->identify();
      break;
    case 'M':
      rs = true;
      r = b->wait();
      break;
    case 'D':
      r = b->data();
      break;
    case 'A':
      addr = buf[2];
      b->addr = addr;
      EEPROM.write(EE_ADDR+i, addr);
      break;
    }

  String s = String(addr) + r + "\r\n";
  socket.sendResponse(s);
  if (rs)
    b->readSample();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < NUM_CON; i++)
    blk[i].addr = peekaddr(i);
  socket.begin();
  delay(500);
  socket.forceListen();
}

void loop() {
  if (socket.available()) {
    char c = socket.read();
    if (c == '!') {
      digitalWrite(LED_BUILTIN, HIGH);
      socket.clearBuffer();
      socket.forceHold();
      if (len > 0) {
        rc();
        len = 0;
      }
      socket.forceListen();
      digitalWrite(LED_BUILTIN, LOW);
    } else if (c > 0 && len < CMD_LEN)
      buf[len++] = c;
  }
}
