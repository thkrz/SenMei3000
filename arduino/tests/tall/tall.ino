#include <RTCZero.h>
#include <SDI12.h>
#include <SPI.h>

#include "config.h"
#include "global.h"

#define CAP 1024
#define MSG 12

#define FET 6 /* LED_BUILTIN */
#define MX  1
#define RX  4
#define TX  3
#define MOD 5
#define CS  7

#define BLK (sizeof(uint32_t))
#define LF "\r\n"
#define LEN (addr[0])
#define WAKE 0
#define SIGN(x) ((x)>=0?'+':'\0')

RTCZero rtc;
SDI12 socket(MX, RX, TX);
uint32_t addr[CAP];
char sid[63];
String q;

float battery() {
  analogRead(A1);
  int p = analogRead(A1);
  return (float)p * 0.014956;  // R1 = 1.2M; R2 = 330k
}

void die() {
  disable();
  for(;;)
    delay(1000);
}

void disable() {
  digitalWrite(FET, LOW);
}

void enable() {
  digitalWrite(FET, HIGH);
  delay(650);
}

bool handshake(char i) {
  static char cmd[3] = "a!";

  cmd[0] = i;
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd, WAKE);
    delay(30);
    if (socket.available()) {
      socket.clearBuffer();
      return true;
    }
  }
  socket.clearBuffer();
  return false;
}

String& ident(char i) {
  static char cmd[4] = "aI!";

  cmd[0] = i;
  socket.sendCommand(cmd, WAKE);
  delay(30);
  return readline();
}

String& measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";

  st[0] = i;
  socket.sendCommand(st, WAKE);
  delay(30);
  String s = readline();
  uint8_t wait = s.substring(1, 4).toInt();
  //uint8_t num = s.charAt(4) - '0';

  for (int j = 0; j <= wait; j++) {
    if (socket.available()) {
      socket.clearBuffer();
      break;
    }
    delay(1000);
  }

  rd[0] = i;
  socket.sendCommand(rd, WAKE);
  delay(30);
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

String& readline() {
  static String s;

  s = "";
  while (socket.available() > 0) {
    char c = socket.read();
    s += c;
    if (c == '\n')
      break;
    delay(7);
  }
  socket.clearBuffer();
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

void schedule() {
  uint8_t m = (rtc.getMinutes() / MIVL + 1) * MIVL;
  if (m == 60)
    m = 0;
  rtc.setAlarmMinutes(m);
}

bool update() {
  String s = "UPDATE\r\n";
  for (char *p = sid; *p; p++)
    s += ident(*p);
  return post(s);
}

void setup() {
  pinMode(FET, OUTPUT);
  digitalWrite(FET, LOW);

  Serial.begin(9600);
  while (!Serial);
  delay(3000);

  //pullup();

  Serial.print("SCAN...");
  socket.begin();
  enable();
  scan();
  Serial.println("DONE");
  Serial.print("FOUND: ");
  Serial.println(sid);
  if (sid[0] == '\0')
    die();

  update();
  disable();

  q.reserve(256);

  rtc.begin();

  //rtc.setAlarmSeconds(0);
  //rtc.enableAlarm(rtc.MATCH_MMSS);
  //schedule();

  //rtc.standbyMode();
}

void loop() {
  q = "";
  q += rtc.getYear() + 2000;
  q += '-';
  q += prnt2(rtc.getMonth());
  q += '-';
  q += prnt2(rtc.getDay());
  q += 'T';
  q += prnt2(rtc.getHours());
  q += ':';
  q += prnt2(rtc.getMinutes());
  q += LF;

  float bat0 = battery();
  q += '%';
  q += SIGN(bat0);
  q += bat0;
  q += LF;

  enable();
  for (char *p = sid; *p; p++)
    q += measure(*p);
  disable();

  post(q);

  //schedule();
  //rtc.standbyMode();
  delay(10000);
}
