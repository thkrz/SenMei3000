#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <EEPROM.h>
#include <SDI12.h>

#define SLEEP_TIMEOUT 10000

#define CMD_LEN 4
#define EE_ADDR 0
#define BUS_PIN 7
#define MAX_RSP 72

char peekaddr();
void readSample();
void rc();

SDI12 socket(BUS_PIN);
char addr;
char sample[72];
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

void readSample() {
}

void rc() {
  static char rsp[MAX_RSP];

  char a = cmd[0];
  if (addr != a)
    return;
  rsp[0] = a;
  bool rs = false;
  if (len > 1)
    switch (cmd[1]) {
    case 'I':
      strcpy(&rsp[1], "13JMUWUERZKLIM100001");
      break;
    case 'M':
      rs = true;
      strcpy(&rsp[1], "0026");
      break;
    case 'D':
      strcpy(&rsp[1], sample);
      break;
    case 'A':
      a = cmd[2];
      addr = a;
      EEPROM.write(EE_ADDR, addr);
      break;
    }

  strcat(rsp, "\r\n");
  socket.sendResponse(rsp);
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
  socket.forceListen();
  //sleep();
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
