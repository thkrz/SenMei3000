#include <SDI12.h>
#include <SPI.h>

#include "config.h"
#include "global.h"

#define CAP 1024
#define MSG 12

#define FET 0
#define MX  1
#define RX  4
#define TX  3

#define BLK (sizeof(uint32_t))
#define LF "\r\n"
#define LEN (addr[0])
#define WAKE_DELAY 0
#define SIGN(x) ((x)>=0?'+':'\0')

void disable();
void enable();
String& ident(char);
String& measure(char);
bool post(String&);
void pullup();
String& readline(uint32_t timeout = SDI_TIMEOUT);
bool update();

SDI12 socket(MX, RX, TX);
char *sid = "01c";
String q;

void disable() {
  digitalWrite(FET, LOW);
}

void enable() {
  digitalWrite(FET, HIGH);
  delay(600);
}

String& ident(char i) {
  static char cmd[4] = "aI!";

  cmd[0] = i;
  socket.sendCommand(cmd, WAKE_DELAY);
  return readline();
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

//int nval(String &s) {
//  int n = 0;
//  for (char *p = s.c_str(); *p; p++)
//    if (*p == '+' || *p == '-')
//      n++;
//  return n;
//}

char *prnt2(uint8_t n) {
  static char buf[3];

  sprintf(buf, "%02d", n);
  return buf;
}

bool post(String &s) {
  int n = s.length();
  if (n == 0)
    return false;
  Serial.println(F("POST "PATH"/"STAT_CTRL_ID" HTTP/1.1"));
  Serial.println(F("Host: "HOST));
  Serial.println(F("Connection: close"));
  Serial.println(F("Content-Type: text/plain"));
  Serial.print(F("Content-Length: "));
  Serial.println(n);
  Serial.println();
  Serial.print(s);
  return true;
}

void pullup() {
  static int8_t pin[8] = {
    A0, A2, A3, A4, A5, A6, 0, 5
  };

  for (int i = 0; i < 8; i++)
    pinMode(pin[i], INPUT_PULLUP);
}

String& readline(uint32_t timeout) {
  static String s;

  s = "";
  uint32_t st = millis();
  while ((millis() - st) < timeout) {
    if (socket.available()) {
      char c = socket.read();
      s += c;
      if (c == '\n')
        break;
    } else
      delay(7);
  }
  socket.clearBuffer();
  return s;
}

bool update() {
  String s = "UPDATE\r\n";
  enable();
  for (char *p = sid; *p; p++)
    s += ident(*p);
  disable();
  return post(s);
}

void setup() {
  pinMode(FET, OUTPUT);
  digitalWrite(FET, LOW);
  delay(5000);

  Serial.begin(19200);
  while (!Serial)
    ;

  Serial.println("START...");
  pullup();


  socket.begin();
  update();

  q.reserve(256);
}

void loop() {
  q = "";
  enable();
  for (char *p = sid; *p; p++)
    q += measure(*p);
  disable();
  post(q);
  delay(15000);
}
