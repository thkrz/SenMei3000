#include <MKRNB.h>
#include <RTCZero.h>
#include <SDI12.h>
#include <SHTC3.h>
#include <SPI.h>
#include <SPIMemory.h>
#include <Wire.h>

#include "config.h"

#define FET 0
#define MX  1
#define RX  4
#define TX  3
#define CS  7

#define BSZ 4 /* (sizeof(uint32_t)) */
#define CAP 1024
#define LF "\r\n"
#define LEN (addr[0])
#define WAKE_DELAY 0
#define SIGN(x) ((x)>=0?'+':'\0')

static float battery();
static void connect();
static void ctrl();
static void die();
static void dir();
static void disable();
static void discard();
static bool dump(String&);
static void enable();
static void erase();
static bool handshake(char);
static String& ident(char);
static bool load(String&);
static String& measure(char);
static char *prnt2(uint8_t);
static bool post(String&);
static void pullup();
static String& readline(uint32_t timeout = SDI_TIMEOUT);
static void resend();
static void scan();
static void schedule();
static void sync();
static bool update();
static void verify();

GPRS gprs;
NBClient client;
NB nbAccess;
RTCZero rtc;
SDI12 socket(MX, RX, TX);
SPIFlash flash(CS);
char sid[63];
String q;
uint32_t addr[CAP];

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

void ctrl() {
  uint32_t len;
  int n;
  String s;

  Serial.begin(19200);
  while(!Serial);

  for (;;) {
    if (Serial.available()) {
      char c = Serial.read();
      switch (c) {
      case 'd':
        len = LEN;
        while (load(s)) {
          Serial.print(s);
          discard();
        }
        LEN = len;
        Serial.println(F("#DUMP"));
        break;
      case 'f':
        erase();
        Serial.println(F("#ERASE"));
        break;
      case 'i':
        Serial.print(F("M IN CACHE: "));
        Serial.println(LEN);
        n = 0;
        for (int i = 1; i < CAP; i++)
          if (addr[i] > 0)
            n++;
          else
            break;
        Serial.print(F("M ON FLASH: "));
        Serial.println(n);
        Serial.println(F("#INFO"));
        break;
      case '?':
        Serial.println(F("d: dump"));
        Serial.println(F("i: info"));
        Serial.println(F("f: erase"));
        Serial.println(F("?: help"));
        break;
      }
    }
    delay(10);
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

void dir() {
  for (int i = 0; i < CAP; i++)
    addr[i] = flash.readULong(i*BSZ);
}

void disable() {
  digitalWrite(FET, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

void discard() {
  if (LEN > 0)
    LEN--;
}

bool dump(String &s) {
  if (LEN < CAP-1) {
    uint32_t a = flash.getAddress(flash.sizeofStr(s));
    if (a == 0)
      return false;
    addr[LEN+1] = a;
    if (flash.writeStr(a, s)) {
      LEN++;
      sync();
      return true;
    }
  }
  return false;
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(FET, HIGH);
  delay(600);
}

void erase() {
  flash.eraseChip();
  for (int i = 0; i < CAP; i++) {
    flash.writeULong(i*BSZ, 0);
    addr[i] = 0;
  }
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

bool load(String &s) {
  if (LEN < 1)
    return false;
  return flash.readStr(addr[LEN], s);
}

String& measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";

  st[0] = i;
  socket.sendCommand(st, WAKE_DELAY);
  String s = readline();
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

char *prnt2(uint8_t n) {
  static char buf[3];

  sprintf(buf, "%02d", n);
  return buf;
}

bool post(String &s) {
  int n = s.length();
  if (client.connect(HOST, PORT)) {
    client.println(F("POST "PATH"/"STAT_CTRL_ID" HTTP/1.1"));
    client.println(F("Host: "HOST));
    client.println(F("Connection: close"));
    client.println(F("Content-Type: text/plain; charset=utf-8"));
    client.print(F("Content-Length: "));
    client.println(n);
    client.println();
    client.print(s);

    char buf[HTTP_MSG_LEN];
    uint32_t st = millis();
    for (n = 0; n < HTTP_MSG_LEN && (millis() - st) < HTTP_TIMEOUT; )
      if (client.available())
        buf[n++] = client.read();
      else
        delay(100);
    if (HTTP_OK(buf, n))
      return true;
  }
  return false;
}

void pullup() {
  int8_t pin[9] = {
    A0, A2, A3, A4, A5, A6, 5, 13, 14 // 2
  };

  for (int i = 0; i < 9; i++)
    pinMode(pin[i], INPUT_PULLUP);
}

String& readline(uint32_t timeout) {
  static String s;

  s = "";
  uint32_t st = millis();
  while ((millis() - st) < timeout) {
    if (socket.available()) {
      char c = socket.read();
      if (c == 0)
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

void resend() {
  String s;
  while (load(s)) {
    if (!post(s))
      break;
    discard();
    sync();
  }
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
#if defined(MI_MINUTE)
  uint8_t m = (rtc.getMinutes() / MI_MINUTE + 1) * MI_MINUTE;
  if (m == 60)
    m = 0;
  rtc.setAlarmMinutes(m);
#elif defined(MI_HOUR)
  uint8_t m = (rtc.getHours() / MI_HOUR + 1) * MI_HOUR;
  rtc.setAlarmHours(m % 24);
#endif
}

void sync() {
  while (!flash.eraseSector(0));
  for (int i = 0; i < CAP; i++)
    flash.writeULong(i*BSZ, addr[i]);
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
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FET, OUTPUT);
  disable();

  pullup();

  flash.begin();
  dir();

  if (battery() < 7)
    ctrl();
    /* not reached */

  flash.powerDown();

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
#if defined(MI_MINUTE)
  rtc.enableAlarm(rtc.MATCH_MMSS);
#elif defined(MI_HOUR)
  rtc.setAlarmMinutes(0);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
#else
  die();
#endif
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

  flash.powerUp();
  delay(1);
  if (pm) {
    nbAccess.shutdown();
    dump(q);
  } else {
    verify();
    if (LEN > 0)
      resend();
    if (!post(q))
      dump(q);
  }
  flash.powerDown();

#if defined(MI_MINUTE)
  if (!pm)
#endif
    schedule();
  rtc.standbyMode();
}
