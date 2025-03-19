#include <EEPROM.h>
#include <SDI12.h>

#include "Block.h"

#define CMD_LEN 4
#define EE_ADDR 0
#define NUM_CON 6
#define BUS_PIN 11

static int index(char);
static char peekaddr(int);
static void rc();

Block blk[NUM_CON] = {
  Block(&SMT100, 7, A0, A1), // 1
  Block(&SMT100, 5, A2, A3), // 2
  Block(&SMT100, 3, A4, A5), // 3
  Block(&SMT100, 2, A4, A5), // 3
  Block(&SMT100, 1, A8, A9), // 5
  Block(&SMT100, 0, A10, A11)// 6
};
SDI12 socket(BUS_PIN);
char cmd[CMD_LEN];
int len = 0;

int index(char a) {
  for (int i = 0; i < NUM_CON; i++)
    if (blk[i].addr == a)
      return i;
  return -1;
}

char peekaddr(int a) {
  char c = EEPROM.read(EE_ADDR + a);
  if (!isAlphaNumeric(c)) {
    c = '0' + a;
    EEPROM.write(EE_ADDR + a, c);
  }
  return c;
}

void rc() {
  char addr = cmd[0];
  int i = index(addr);
  if (i < 0)
    return;
  Block *b = &blk[i];
  if (!b->isConnected())
    return;
  String r = "";
  bool rs = false;
  if (len > 1)
    switch (cmd[1]) {
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
      addr = cmd[2];
      b->addr = addr;
      EEPROM.write(EE_ADDR+i, addr);
      break;
    }

  String s;
  s += addr;
  s += r;
  s += "\r\n";
  socket.sendResponse(s);
  if (rs)
    b->readSample();
}

void setup() {
  for (int i = 0; i < NUM_CON; i++)
    blk[i].addr = peekaddr(i);
  socket.begin();
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
      cmd[len++] = c;
  }
}
