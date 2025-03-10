#include <SDI12.h>
#include <SPI.h>

#define FET 0 /* LED_BUILTIN */
#define MX  1
#define RX  4
#define TX  3

#define LF "\r\n"
#define WAKE_DELAY 0
#define SDI_TIMEOUT 3000

void disable();
void enable();
String& readline(uint32_t timeout = SDI_TIMEOUT);

SDI12 socket(MX, RX, TX);
char sid[63];

void disable() {
  digitalWrite(FET, LOW);
}

void enable() {
  digitalWrite(FET, HIGH);
  delay(600);
}

bool handshake(char i) {
  static char cmd[3] = "a!";

  cmd[0] = i;
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd, WAKE_DELAY);
    String s = readline(50);
    if (s.charAt(0) == i) {
      return true;
    }
  }
  return false;
}

String& measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";

  st[0] = i;
  socket.sendCommand(st, WAKE_DELAY);
  String s = readline();
  uint8_t wait = s.substring(1, 4).toInt();
  //uint8_t num = s.charAt(4) - '0';

  for (int j = 0; j < wait; j++) {
    if (socket.available()) {
      socket.clearBuffer();
      break;
    }
    delay(1000);
  }

  rd[0] = i;
  socket.sendCommand(rd, WAKE_DELAY);
  return readline();
}

String& readline(uint32_t timeout) {
  static String s;

  s = "";
  uint32_t st = millis();
  while ((millis() - st) < SDI_TIMEOUT) {
    if (socket.available()) {
      char c = socket.read();
      s += c;
      if (c == '\n')
        break;
    } else
      delay(10);
  }
  socket.clearBuffer();
  return s;
}

void scan() {
  int n = 0;
  for (char c = '0'; c <= '1'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  for (char c = 'c'; c <= 'd'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  sid[n] = '\0';
}

void setup() {
  pinMode(FET, OUTPUT);
  disable();

  socket.begin();
  delay(500);
  scan();
}

void loop() {
  String s;
  static char cmd[4] = "aI!";

  enable();
  for (char *p = sid; *p; p++) {
    cmd[0] = *p;
    socket.sendCommand(cmd);
    s = readline();
    delay(1000);
  }
  disable();

  delay(10000);
}
