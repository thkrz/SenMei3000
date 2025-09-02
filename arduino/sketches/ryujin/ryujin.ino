#include <RTCZero.h>
#include <SDI12.h>
#include <SHTC3.h>
#include <Wire.h>

#include "W25QLOG.h"
#include "config.h"
#include "gsm.h"

#define FET 0
#define MX 1
#define RX 4
#define TX 3
#define CS 7

#define LF "\r\n"
#define WAKE_DELAY 0
#define SIGN(x) ((x) >= 0 ? "+" : "")

typedef String& (*command)(char);

static float battery();
static bool config();
static bool connect();
static void ctrl();
static void die(uint32_t);
static void disable();
static void disconnect();
static void enable();
static bool handshake(char);
static String& ident(char);
static String& measure(char);
static bool ping();
static bool post(String&);
static void pullup();
static void pulse(int, uint32_t);
static String& rc(command, char);
static String& readline(uint32_t timeout = SDI_TIMEOUT);
static bool reconnect();
static bool resend();
static void scan();
static void schedule();
static void settime();
static bool valid(char);
static bool wait(uint32_t timeout = MODEM_TIMEOUT);

RTCZero rtc;
SDI12 socket(MX, RX, TX);
W25QLOG w25q(CS);
TinyGsm modem(SerialSARA);
TinyGsmClient client(modem);
char sid[63];
String q;
bool power;

float battery() {
  analogRead(A1);
  int p = analogRead(A1);
  return (float)p * 0.014956;  // R1 = 1.2M; R2 = 330k
}

bool config() {
  String s = "CONFIG\r\n";
#if defined(MI_MINUTE)
  uint16_t i = MI_MINUTE * 60;
#elif defined(MI_HOUR)
  uint16_t i = MI_HOUR * 3600;
#endif
  s += i;
  s += LF;
  enable();
  for (char* p = sid; *p; p++)
    s += rc(ident, *p);
  disable();
  return post(s);
}

bool connect() {
  SerialSARA.begin(115200);
  pulse(SARA_PWR_ON, 200);
  if (!wait() || !modem.init() || !modem.waitForNetwork())
    return false;
  if (!modem.gprsConnect(APN))
    return false;
  power = true;
  return true;
}

void ctrl() {
  String s;
  s.reserve(256);

  Serial.begin(19200);
  while (!Serial);

  for (;;) {
    if (Serial.available()) {
      char c = Serial.read();
      switch (c) {
        case 'd':
          w25q.seek(0);
          while (w25q.read(s))
            Serial.print(s);
          break;
        case 'f':
          w25q.format();
          Serial.print("chip formatted\r\n");
          break;
        case 'i':
          Serial.print(F("FIRMWARE: " FIRMWARE "\r\n"));
          Serial.print(F("STAT_CTRL_ID: " STAT_CTRL_ID "\r\n"));
          Serial.print(F("APN: " APN "\r\n"));
#if defined(MI_MINUTE)
          Serial.print(F("MI_MINUTE: "));
          Serial.print(MI_MINUTE);
          Serial.print(F("\r\n"));
#elif defined(MI_HOUR)
          Serial.print(F("MI_HOUR: "));
          Serial.print(MI_HOUR);
          Serial.print(F("\r\n"));
#endif
          break;
      }
      Serial.print(F("#"));
    }
  }
}

void die(uint32_t p) {
  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(p);
    digitalWrite(LED_BUILTIN, LOW);
    delay(p);
  }
}

void disable() {
  digitalWrite(FET, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  socket.end();
}

void disconnect() {
  client.stop();

  if (modem.isGprsConnected())
    modem.gprsDisconnect();
  if (modem.poweroff())
    delay(1500);
  SerialSARA.end();
  power = false;
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(FET, HIGH);
  socket.begin();
  delay(600);
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

bool ping() {
  TinyGsmClient probe(modem);
  if (probe.connect("8.8.8.8", 53)) {
    probe.stop();
    return true;
  }
  return false;
}

bool post(String& s) {
  if (!client.connect(HOST, PORT))
    return false;

  int n = s.length();
  client.println(F("POST " PATH "/" STAT_CTRL_ID " HTTP/1.1"));
  client.println(F("Host: " HOST));
  client.println(F("Connection: close"));
  client.println(F("Content-Type: text/plain; charset=utf-8"));
  client.print(F("Content-Length: "));
  client.println(n);
  client.println();
  client.print(s);

  n = 0;
  char r = 0;
  uint32_t timeout = millis();
  while (n < HTTP_MSG_LEN && (millis() - timeout) < HTTP_TIMEOUT)
    if (client.available()) {
      r = client.read();
      n++;
    }

  /* flush */
  while (client.available())
    client.read();

  client.stop();
  return n == HTTP_MSG_LEN && r == '2';
}

void pullup() {
  int8_t pin[10] = {
    A0, A2, A3, A4, A5, A6, 2, 5, 13, 14
  };

  for (int i = 0; i < 8; i++)
    pinMode(pin[i], INPUT_PULLUP);
}

void pulse(int pin, uint32_t len) {
  digitalWrite(pin, HIGH);
  delay(len);
  digitalWrite(pin, LOW);
}

String& rc(command c, char i) {
  String* s = &c(i);
  if (s->length() == 0) {
    *s += i;
    *s += LF;
  }
  return *s;
}

String& readline(uint32_t timeout) {
  static String s;

  s = "";
  uint32_t st = millis();
  while ((millis() - st) < timeout)
    if (socket.available()) {
      char c = socket.read();
      if (!valid(c))
        continue;
      s += c;
      if (c == '\n') {
        socket.clearBuffer();
        return s;
      }
    }
  socket.clearBuffer();
  s = "";
  return s;
}

bool reconnect() {
  if (!power)
    return connect();

  if (modem.isNetworkConnected() && modem.isGprsConnected() && ping())
    return true;

  for (uint8_t j = 0; j < 3; j++) {
    if (!modem.testAT()) {
      pulse(SARA_RESETN, 150);
      wait();
      modem.init();
    }
    if (modem.waitForNetwork()) {
      modem.gprsDisconnect();
      if (modem.gprsConnect(APN) && ping())
        return true;
    }
    modem.restart();
  }
  return false;
}

bool resend() {
  static String s;

  while (w25q.read(s)) {
    if (!post(s))
      return false;
    w25q.unlink();
  }
  return true;
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
  int Y, m, d, h, b, s;
  float z;

  if (modem.getNetworkTime(&Y, &m, &d, &h, &b, &s, &z)) {
    rtc.setDate(d, m, (Y - 2000));
    rtc.setTime(h, b, s);
  }
}

bool valid(char c) {
  return (isPrintable(c) || c == '\r' || c == '\n');
}

bool wait(uint32_t timeout) {
  delay(2000);
  uint32_t st = millis();
  while (millis() - st < timeout) {
    if (modem.testAT())
      return true;
    delay(500);
  }
  return false;
}

void setup() {
  pinMode(FET, OUTPUT);
  digitalWrite(FET, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, LOW);
  pinMode(SARA_PWR_ON, OUTPUT);
  digitalWrite(SARA_PWR_ON, LOW);

  pullup();

  w25q.begin();
  if (battery() < 7)
    ctrl();
  /* not reached */
  w25q.sleep(true);

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
  for (char* p = sid; *p; p++)
    q += rc(measure, *p);
  disable();

  w25q.sleep(false);
  if (psm) {
    if (power)
      disconnect();
    w25q.append(q);
  } else if (!reconnect() || !resend() || !post(q))
    w25q.append(q);
  w25q.sleep(true);

#if defined(MI_MINUTE)
  if (!psm)
#endif
    schedule();
  rtc.standbyMode();
}
