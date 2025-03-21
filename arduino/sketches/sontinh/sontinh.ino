#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <EEPROM.h>
#include <SDI12.h>

#define SLEEP_TIMEOUT 10000

#define CMD_LEN 4
#define EE_ADDR 0
#define NUM_CON 6
#define BUS_PIN 7

static String& measurement();
static char peekaddr();
static void readSample();
static void rc();

SDI12 socket(BUS_PIN);
char addr;
char cmd[CMD_LEN];
int len = 0;
uint32_t wait;

char peekaddr() {
  char c = EEPROM.read(EE_ADDR);
  if (!isAlphaNumeric(c)) {
    c = '0' + a;
    EEPROM.write(EE_ADDR, c);
  }
  return c;
}

void rc() {
  char a = cmd[0];
  if (addr != a)
    return;
  String r = "";
  bool rs = false;
  if (len > 1)
    switch (cmd[1]) {
    case 'I':
      r = "13JMUWUERZKLIM100001";
      break;
    case 'M':
      rs = true;
      r = "0026";
      break;
    case 'D':
      r = measurement();
      break;
    case 'A':
      a = cmd[2];
      addr = a;
      EEPROM.write(EE_ADDR, addr);
      break;
    }

  String s;
  s += addr;
  s += r;
  s += "\r\n";
  socket.sendResponse(s);
  if (rs)
    readSample();
}

void sleep() {
  pinMode(BUS_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUS_PIN), wake, RISING);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  sleep_disable();
}

void wake() {
  detachInterrupt(0);
  socket.forceListen();
  wait = millis();
}

void setup() {
  addr = peekaddr();
  socket.begin();
  sleep();
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
      wait = millis();
    } else if (c > 0 && len < CMD_LEN)
      cmd[len++] = c;
  }
  if (millis() - wait > SLEEP_TIMEOUT)
    sleep();
}
