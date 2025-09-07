#include <EEPROM.h>
#include <SDI12.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define SLEEP_TIMEOUT 10000L

#define BUS_PIN 7
#define CMD_LEN 4
#define EE_ADDR 0
#define MAX_RSP 72

enum { INT1 = 0x01, INT2 = 0x02 };
volatile uint8_t interrupt = 0;

void int1ISR();
void int2ISR();
char peekaddr();
void rc();
void sample();
void sleep();

SDI12 socket(BUS_PIN);
char addr;
char cmd[CMD_LEN];
char measurement[MAX_RSP - 3];
int len = 0;
uint32_t wait;

void int1ISR() { interrupt |= INT1; }
void int2ISR() { interrupt |= INT2; }

char peekaddr() {
  char c = EEPROM.read(EE_ADDR);
  if (!isAlphaNumeric(c)) {
    c = '0';
    EEPROM.write(EE_ADDR, c);
  }
  return c;
}

void rc() {
  static char rsp[MAX_RSP];

  char a = cmd[0];
  if (a == '?')
    a = addr;
  else if (addr != a)
    return;
  rsp[0] = a;
  rsp[1] = '\0';
  bool rs = false;
  if (len > 1)
    switch (cmd[1]) {
    case 'I':
      strcpy(&rsp[1], F("13JMUWUERZKLIM100001"));
      break;
    case 'M':
      rs = true;
      strcpy(&rsp[1], F("0026"));
      break;
    case 'D':
      strcpy(&rsp[1], measurement);
      break;
    case 'A':
      if (len < 2)
        return;
      a = cmd[2];
      addr = a;
      EEPROM.write(EE_ADDR, addr);
      break;
    }

  strcat(rsp, F("\r\n"));
  socket.sendResponse(rsp);
  if (rs)
    sample();
}

void sample() {}

void sleep() {
  socket.forceListen();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  noInterrupts();
#if defined(BODS) && defined(BODSE)
  // Disable brown-out during sleep for lower current (if supported)
  MCUCR = MCUCR | _BV(BODSE) | _BV(BODS);
  MCUCR = (MCUCR & ~_BV(BODSE)) | _BV(BODS);
#endif
  interrupts();

  sleep_cpu();
  sleep_disable();
}

void setup() {
  addr = peekaddr();

  socket.begin();
  delay(100);
  socket.forceListen();

  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(0), int1ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(1), int2ISR, LOW);

  sleep();
}

void loop() {
  if (socket.available()) {
    wait = millis();
    char c = socket.read();
    if (c == '!') {
      socket.clearBuffer();
      socket.forceHold();
      if (len > 0) {
        rc();
        len = 0;
      }
    } else if (c > 0 && len < CMD_LEN)
      cmd[len++] = c;
  }
  socket.forceListen();

  noInterrupts();
  uint8_t i = interrupt;
  interrupt = 0;
  interrupts();

  if (i & INT1) {
    wait = millis();
  }
  if (i & INT2) {
    wait = millis();
  }

  if (millis() - wait > SLEEP_TIMEOUT)
    sleep();
}
