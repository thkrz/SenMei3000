#include <MKRNB.h>
#include <RTCZero.h>
#include <SDI12.h>
#include <SHTC3.h>
#include <SPI.h>
#include <SPIMemory.h>
#include <Wire.h>

#include "config.h"
#include "global.h"

#define CAP 1024
#define MSG 12

#define FET 0
#define MX  1
#define RX  4
#define TX  3
#define MOD 5
#define CS  7

#define BSZ (sizeof(uint32_t))
#define LF "\r\n"
#define WAKE_DELAY 0
#define SIGN(x) ((x)>=0?'+':'\0')

static float battery();
static void connect();
static void die();
static void disable();
static void enable();
static bool handshake(char);
static String& ident(char);
static String& measure(char);
static char *prnt2(uint8_t);
static bool post(String&);
static void pullup();
static String& readline(uint32_t timeout = SDI_TIMEOUT);
static void scan();
static void schedule();
static bool update();
static void verify();

GPRS gprs;
NBClient client;
NB nbAccess;
RTCZero rtc;
SDI12 socket(MX, RX, TX);
char sid[63];
String q;

float battery() {
  analogRead(A1);
  int p = analogRead(A1);
  return (float)p * 0.014956;  // R1 = 1.2M; R2 = 330k
}

void connect() {
  bool connected = false;
  while (!connected) {
    if ((nbAccess.begin() == NB_READY)
        && (gprs.attachGPRS() == GPRS_READY)) {
      connected = true;
    } else {
      delay(1000);
    }
  }
}

void die() {
  for(;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}

void disable() {
  digitalWrite(LED_BUILTIN, LOW);
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
}

bool handshake(char i) {
  static char cmd[3] = "a!";

  cmd[0] = i;
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd, WAKE_DELAY);
    String s = readline(50);
    if (s.charAt(0) == i)
      return true;
  }
  return false;
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
  bool ok = false;
  if (client.connect(HOST, PORT)) {
    client.println(F("POST "PATH"/"STAT_CTRL_ID" HTTP/1.1"));
    client.println(F("Host: "HOST));
    client.println(F("Connection: close"));
    client.println(F("Content-Type: text/plain"));
    client.print(F("Content-Length: "));
    client.println(n);
    client.println();
    client.print(s);

    char buf[MSG];
    uint32_t st = millis();
    for (int i = 0; i < MSG; i++) {
      while (client.available() < 0 && (millis() - st) < HTTP_TIMEOUT)
        delay(10);
      buf[i] = client.read();
    }
    ok = strncmp("HTTP/1.1 201", buf, MSG) == 0;
  }
  return ok;
}

void pullup() {
  int8_t pin[7] = {
    A0, A2, A3, A4, A5, A6, 5
  };

  for (int i = 0; i < 7; i++)
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

void scan() {
  int n = 0;
  for (char c = '0'; c <= '2'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  for (char c = 'a'; c <= 'c'; c++) {
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
  enable();
  for (char *p = sid; *p; p++)
    s += ident(*p);
  disable();
  return post(s);
}

void verify() {
  //if (!client.connected())
  //  client.stop();
  if (!nbAccess.isAccessAlive()) {
    nbAccess.shutdown();
    connect();
  }
}

void setup() {
  pinMode(FET, OUTPUT);
  digitalWrite(FET, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  disable();

  pullup();

  socket.begin();
  enable();
  scan();
  disable();
  if (sid[0] == '\0')
    die();

  connect();
  update();

  Wire.begin();
  SHTC3.begin();

  q.reserve(256);

  rtc.begin();
  rtc.setEpoch(nbAccess.getTime());

  rtc.setAlarmSeconds(0);
  rtc.enableAlarm(rtc.MATCH_MMSS);
  schedule();

  rtc.standbyMode();
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

  bool pm = bat0 < BAT_LOW;

  SHTC3.readSample(true, pm);
  float st = SHTC3.getTemperature();
  float rh = SHTC3.getHumidity();
  q += SIGN(st);
  q += st;
  q += SIGN(rh);
  q += rh;
  q += LF;

  enable();
  for (char *p = sid; *p; p++)
    q += measure(*p);
  disable();

  verify();
  post(q);

  schedule();
  rtc.standbyMode();
}
