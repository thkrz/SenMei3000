#include <SDI12.h>
#include <SPI.h>

#define MX  1
#define RX  4
#define TX  3

#define LF "\r\n"
#define WAKE_DELAY 0
#define SDI_TIMEOUT 3000

String& readline(uint32_t timeout = SDI_TIMEOUT);

//SDI12 socket(MX, RX, TX);
SDI12 socket(9);
char cmd[128];
int len = 0;


//int nval(String &s) {
//  int n = 0;
//  for (char *p = s.c_str(); *p; p++)
//    if (*p == '+' || *p == '-')
//      n++;
//  return n;
//}

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

void setup() {
  Serial.begin(19200);
  while (!Serial);

  socket.begin();
  delay(500);
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
    case '\r':
      Serial.println();
      if (len > 1 && cmd[len-1] == '!') {
        cmd[len] = '\0';
        socket.sendCommand(cmd, WAKE_DELAY);
        delay(30);
        Serial.println("COMMAND SENT");
      }
      len = 0;
      break;
    default:
      Serial.write(c);
      if (len < 127)
        cmd[len++] = c;
      break;
    }
  }
}
