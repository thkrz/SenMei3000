#include <SDI12.h>
#include <SPI.h>

SDI12 socket(1);
char sid[63];

void blink(int n) {
  for(int i = 0; i < n; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}

void disable() {
  socket.end();
  digitalWrite(0, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(0, HIGH);
  socket.begin();
  delay(500);
}

bool handshake(char i) {
  static char cmd[3] = "0!";

  cmd[0] = i;
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd);
    socket.clearBuffer();
    delay(30);
    if (socket.available())
      return true;
  }
  socket.clearBuffer();
  return false;
}

String measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";
  static String s;

  socket.clearBuffer();
  delay(10);
  st[0] = i;
  socket.sendCommand(st);
  delay(30);
  s = socket.readStringUntil('\n');
  uint8_t wait = s.substring(1, 4).toInt();

  for (int j = 0; j < wait; j++) {
    if (socket.available()) {
      socket.clearBuffer();
      break;
    }
    delay(1000);
  }

  rd[0] = i;
  socket.sendCommand(rd);
  delay(30);
  s = socket.readStringUntil('\n');
  return s;
}

void scan() {
  int n = 0;
  for (char c = '0'; c <= '9'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  for (char c = 'a'; c <= 'z'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  sid[n] = '\0';
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(1, INPUT);
  for (int i = 2; i < 8; i++)
    pinMode(i, INPUT_PULLUP);

  enable();
  scan();
  disable();

  if (sid[0] == '\0')
    blink(1000);
}

void loop() {
  delay(5000);
  String s = "";
  enable();
  for (char *p = sid; *p; p++)
    s += measure(*p);
  disable();
  if (s.length() > 3)
    blink(4);
}
