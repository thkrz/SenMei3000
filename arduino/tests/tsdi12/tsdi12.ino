#include <SDI12.h>
#include <SPI.h>

#define FET 0
#define MX  1
#define RX  4
#define TX  3

#define LF "\r\n"
#define WAKE_DELAY 0
#define SDI_TIMEOUT 3000

String& readline(uint32_t timeout = SDI_TIMEOUT);

SDI12 socket(MX, RX, TX);
//SDI12 socket(9);
char cmd[128];
int len = 0;


String& measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";

  st[0] = i;
  socket.sendCommand(st, WAKE_DELAY);
  String s = readline();
  Serial.print(s);
  uint8_t wait = s.substring(1, 4).toInt();
  //uint8_t num = s.charAt(4) - '0';

  for (int j = 0; j <= wait; j++) {
    if (socket.available() && socket.read() == i)
      break;
    delay(1000);
  }
  socket.clearBuffer();
  rd[0] = i;
  socket.sendCommand(rd, WAKE_DELAY);
  return readline();
}

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
      if (c > 0)
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
  digitalWrite(FET, HIGH);

  Serial.begin(19200);
  while (!Serial);

  socket.begin();
  delay(500);
}

void loop() {
  String s;

  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
    case '\r':
      Serial.println();
      if (len > 1 && cmd[len-1] == '!') {
        cmd[len] = '\0';
        Serial.println("RESP:");
        if (cmd[1] == 'M') {
          s = measure(cmd[0]);
        } else {
          socket.sendCommand(cmd, WAKE_DELAY);
          s = readline();
        }
        Serial.println(s);
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

