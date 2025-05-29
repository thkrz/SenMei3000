#include <RTCZero.h>
#include <SDI12.h>
#include <SHTC3.h>
#include <SPI.h>
#include <SPIMemory.h>
#include <Wire.h>

#include "config.h"
#include "gsm.h"

#define FET 0
#define MX  1
#define RX  4
#define TX  3
#define CS  7

#define BLKSZ 4    /* (sizeof(uint32_t)) */
#define CAP 1024
#define LF "\r\n"
#define LEN (*addr)
#define WAKE_DELAY 0
#define SIGN(x) ((x)>=0?'+':'\0')

static float battery();
static bool config();
static bool connect(bool fastboot = false);
static void ctrl();
static void die(uint32_t);
static void dir();
static void disable();
static void discard();
static void disconnect();
static bool dump(String&);
static void enable();
static void erase();
static bool handshake(char);
static String& ident(char);
static bool load(String&);
static String& measure(char);
static bool pop(String&);
static bool post(String&);
static void powerpulse(uint32_t);
static void pullup();
static String& readline(uint32_t timeout = SDI_TIMEOUT);
static void resend();
static void scan();
static void schedule();
static void settime();
static void sync();
static bool valid(char);
static bool verify();

RTCZero rtc;
SDI12 socket(MX, RX, TX);
SPIFlash flash(CS);
TinyGsm modem(SerialSARA);
TinyGsmClient client(modem);
char sid[63];
String q;
uint32_t addr[CAP];
bool power = false;

float battery() {
  analogRead(A1);
  int p = analogRead(A1);
  return (float)p * 0.014956;  // R1 = 1.2M; R2 = 330k
}

bool config() {
  String s = "CONFIG\r\n";
  enable();
  for (char *p = sid; *p; p++)
    s += ident(*p);
  disable();
  return post(s);
}

bool connect(bool fastboot) {
  SerialSARA.begin(115200);
  powerpulse(150);
  power = true;
  delay(6000);
  if (fastboot)
    modem.init();
  else
    modem.restart();
  if (!modem.waitForNetwork())
    return false;
  return modem.gprsConnect(F(APN));
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
        Serial.println(F("#FORMAT"));
        break;
      }
    }
  }
}

void die(uint32_t p) {
  for(;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(p);
    digitalWrite(LED_BUILTIN, LOW);
    delay(p);
  }
}

void dir() {
  LEN = flash.readULong(0);
  for (int i = 1; i <= LEN; i++)
    addr[i] = flash.readULong(i*BLKSZ);
}

void disable() {
  digitalWrite(FET, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

void discard() {
  if (LEN > 0)
    LEN--;
}

void disconnect() {
  if (modem.isGprsConnected())
    modem.gprsDisconnect();
  modem.poweroff();
  powerpulse(1500);
  power = false;
  SerialSARA.end();
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
    flash.writeULong(i*BLKSZ, 0);
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
  return LEN > 0 && flash.readStr(addr[LEN], s);
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

bool pop(String &s) {
  for (int i = 0; i < 3; i++) {
    if (post(s)) {
      discard();
      sync();
      return true;
    }
    delay(500);
  }
  return false;
}

bool post(String &s) {
  if (!client.connect(HOST, PORT))
    return false;

  int n = s.length();
  client.println(F("POST "PATH"/"STAT_CTRL_ID" HTTP/1.1"));
  client.println(F("Host: "HOST));
  client.println(F("Connection: close"));
  client.println(F("Content-Type: text/plain; charset=utf-8"));
  client.print(F("Content-Length: "));
  client.println(n);
  client.println();
  client.print(s);

  n = 0;
  char buf[HTTP_MSG_LEN];
  uint32_t timeout = millis();
  while (client.connected() || client.available()) {
    while (client.available()) {
      char c = client.read();
      if (n < HTTP_MSG_LEN)
        buf[n++] = c;
    }
    if ((millis() - timeout) > HTTP_TIMEOUT) {
      client.stop();
      return false;
    }
    delay(100);
  }
  client.stop();
  return HTTP_OK(buf, n);
}

void powerpulse(uint32_t len) {
  digitalWrite(SARA_PWR_ON, HIGH);
  delay(len);
  digitalWrite(SARA_PWR_ON, LOW);
}

void pullup() {
  int8_t pin[10] = {
    A0, A2, A3, A4, A5, A6, 2, 5, 13, 14
  };

  for (int i = 0; i < 10; i++)
    pinMode(pin[i], INPUT_PULLUP);
}

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

void resend() {
  static String s = "";

  while (load(s)) {
    if (!pop(s))
      break;
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
  rtc.setAlarmMinutes(m % 60);
#elif defined(MI_HOUR)
  uint8_t m = (rtc.getHours() / MI_HOUR + 1) * MI_HOUR;
  rtc.setAlarmHours(m % 24);
#endif
}

void settime() {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  float tz;

  if (modem.getNetworkTime(&year, &month, &day, &hour, &minute, &second, &tz)) {
    rtc.setDate(day, month, (year-2000));
    rtc.setTime(hour, minute, second);
  }
}

void sync() {
  for (int i = 0; i < (CAP>>10); i++)
    flash.eraseSector(i);
  for (int i = 0; i < CAP; i++)
    flash.writeULong(i*BLKSZ, addr[i]);
}

bool valid(char c) {
  return (isPrintable(c) || c == '\r' || c == '\n');
}

bool verify() {
  if (modem.isNetworkConnected() && modem.isGprsConnected())
    return true;
  if (power) {
    disconnect();
    delay(6000);
  }
  return connect(true);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FET, OUTPUT);
  disable();

  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, LOW);
  pinMode(SARA_PWR_ON, OUTPUT);
  digitalWrite(SARA_PWR_ON, LOW);

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
    die(500);

  Wire.begin();
  SHTC3.begin();

  q.reserve(256);

  if (!connect() || !config())
      die(1500);

  rtc.begin();
  settime();

  rtc.setAlarmSeconds(0);
#if defined(MI_MINUTE)
  rtc.enableAlarm(rtc.MATCH_MMSS);
#elif defined(MI_HOUR)
  rtc.setAlarmMinutes(0);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
#endif
  schedule();
  rtc.standbyMode();
}

void loop() {
  q = "";
  q += rtc.getEpoch();
  q += LF;

  float bat0 = battery();
  q += '%';
  q += SIGN(bat0);
  q += bat0;

  bool psm = bat0 < BAT_LOW;

  SHTC3.readSample(true, psm);
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
  if (psm) {
    if (power)
      disconnect();
    dump(q);
  } else if (verify()) {
    resend();
    if (!post(q))
      dump(q);
  } else
    dump(q);
  flash.powerDown();

#if defined(MI_MINUTE)
  if (!psm)
#endif
    schedule();
  rtc.standbyMode();
}
