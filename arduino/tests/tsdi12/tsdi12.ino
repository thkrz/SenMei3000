#include <SDI12.h>

SDI12 socket(7);
String cmd;

static String& readline(uint32_t timeout = 3000);
static bool valid(char);

String& readline(uint32_t timeout) {
  static String s;

  s = "";
  uint32_t st = millis();
  while ((millis() - st) < timeout) {
    if (socket.available()) {
      char c = socket.read();
      if (!valid(c))
        continue;
      s += c;
      if (c == '\n')
        break;
    } else
      delay(10);
  }
  socket.clearBuffer();
  return s;
}

bool valid(char c) {
  return (isPrintable(c) || c == '\r' || c == '\n');
}

void setup() {
  Serial.begin(19200);
  while (!Serial);

  cmd.reserve(256);

  socket.begin();
  delay(500);
  socket.forceListen();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    cmd += c;
    if (c == '!') {
      socket.sendCommand(cmd);
      Serial.print(readline());
      cmd = "";
    }
  }
}
