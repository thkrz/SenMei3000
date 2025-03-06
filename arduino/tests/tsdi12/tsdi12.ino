#include <SDI12.h>
#include <SPI.h>

#define FET 6 /* LED_BUILTIN */
#define MX  1
#define RX  4
#define TX  3

#define LF "\r\n"
#define WAKE_DELAY 0
#define SDI_TIMEOUT 3000

SDI12 socket(MX, RX, TX);
char cmd[128];
int len;

void disable() {
  digitalWrite(FET, LOW);
}

void enable() {
  digitalWrite(FET, HIGH);
  delay(600);
}

//int nval(String &s) {
//  int n = 0;
//  for (char *p = s.c_str(); *p; p++)
//    if (*p == '+' || *p == '-')
//      n++;
//  return n;
//}

String& readline() {
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

void setup() {
  pinMode(FET, OUTPUT);
  digitalWrite(FET, LOW);

  Serial.begin(19200);
  while (!Serial);
  delay(3000);
  socket.begin();
  len = 0;
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
    case '\r':
      Serial.println();
      if (len > 1 && cmd[len-1] == '!') {
        cmd[len] = '\0';
        enable();
        socket.sendCommand(cmd, WAKE_DELAY);
        String s = readline();
        disable();
        Serial.print(s);
      }
      len = 0;
      break;
    default:
      Serial.print(c);
      if (len < 127)
        cmd[len++] = c;
      break;
    }
  }
}
