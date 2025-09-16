#include <RTCZero.h>
#include <SDI12.h>
#include <SHTC3.h>
#include <Wire.h>

#include "NMEA.h"
#include "W25QLOG.h"
#include "config.h"
#include "gsm.h"

#define FET 0
#define PWR 2
#define MX 1
#define RX 4
#define TX 3
#define CS 7

#define LF "\r\n"
#define WAKE_DELAY 0
#define SIGN(x) ((x) >= 0 ? "+" : "")

typedef String &(*command)(char);

float battery();
bool config();
bool connect();
void ctrl();
void die(uint32_t);
void disable();
void disconnect(bool alive = true);
void enable();
bool gprs();
bool handshake(char);
String &ident(char);
void location();
String &measure(char);
bool post(String &);
void pullup();
void pulse(int, uint32_t);
String &rc(command, char);
String &readline(uint32_t timeout = SDI_TIMEOUT);
bool reconnect();
bool resend();
void scan();
void schedule();
void settime();
void update(float &);
bool valid(char);
bool verify();
bool wait(bool, uint32_t timeout = MODEM_TIMEOUT);

RTCZero rtc;
SDI12 socket(MX, RX, TX);
W25QLOG w25q(CS);
TinyGsm modem(SerialSARA);
TinyGsmClient client(modem);
char sid[63];
String msg;
bool power;

float battery() {
  int p = analogRead(A1);
  return (float)p * 0.014956; // R1 = 1.2M; R2 = 330k
}

bool config() {
  msg = "CONFIG\r\n";
  msg += modem.getIMSI();
  msg += LF;
#if defined(MI_MINUTE)
  uint16_t i = MI_MINUTE * 60;
#elif defined(MI_HOUR)
  uint16_t i = MI_HOUR * 3600;
#endif
  msg += i;
  msg += LF;
  enable();
  for (char *p = sid; *p; p++)
    msg += rc(ident, *p);
  disable();
  return post(msg);
}

bool connect() {
  SerialSARA.begin(115200);
  pulse(SARA_PWR_ON, 1500L);
  power = true;
  if (!wait(MODEM_ON) || !modem.init()) {
    pulse(SARA_RESETN, 50L);
    if (!wait(MODEM_ON) || !modem.init())
      return false;
  }
  if (!modem.waitForNetwork())
    return false;
  return gprs();
}

void ctrl() {
  String s;
  s.reserve(256);

  Serial.begin(19200);
  while (!Serial)
    ;

  for (;;) {
    if (Serial.available()) {
      char c = Serial.read();
      switch (c) {
      case 'd':
        w25q.seek(0);
        while (w25q.read(s, true))
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
#elif defined(MI_HOUR)
        Serial.print(F("MI_HOUR: "));
        Serial.print(MI_HOUR);
#endif
        Serial.print(F("\r\n"));
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

void disconnect(bool alive) {
  client.stop();
  if (alive && modem.isGprsConnected())
    modem.gprsDisconnect();
  if (!(alive && modem.poweroff()))
    pulse(SARA_PWR_ON, 2000L);
  wait(MODEM_OFF);
  power = false;
  SerialSARA.end();
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(FET, HIGH);
  socket.begin();
  delay(600L);
}

bool gprs() {
  uint8_t pause[] = {1, 3, 5};
  for (uint8_t p : pause) {
    if (modem.gprsConnect(APN)) {
      settime();
      return true;
    }
    delay(p * 1000L);
  }
  return false;
}

bool handshake(char i) {
  static char cmd[3] = "a!";

  cmd[0] = i;
  for (uint8_t j = 0; j < 3; j++) {
    socket.sendCommand(cmd, WAKE_DELAY);
    String s = readline(50);
    if (s.charAt(0) == i)
      return true;
  }
  return false;
}

String &ident(char i) {
  static char cmd[4] = "aI!";

  cmd[0] = i;
  socket.sendCommand(cmd, WAKE_DELAY);
  return readline();
}

void location() {
  NMEA nmea;
  digitalWrite(LED_BUILTIN, HIGH);
  nmea.begin();
  int status = 1;
  while (status) {
    status = nmea.poll();
    if (status < 0)
      goto NOT_PRESENT;
  }
NOT_PRESENT:
  nmea.end();
  digitalWrite(LED_BUILTIN, LOW);
}

String &measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";

  st[0] = i;
  socket.sendCommand(st, WAKE_DELAY);
  String s = readline();
  uint8_t wait = s.substring(1, 4).toInt();
  // uint8_t num = s.charAt(4) - '0';

  for (uint8_t j = 0; j <= wait; j++) {
    if (socket.available() && socket.read() == i)
      break;
    delay(1000L);
  }
  socket.clearBuffer();
  rd[0] = i;
  socket.sendCommand(rd, WAKE_DELAY);
  return readline();
}

bool post(String &s) {
  if (!verify())
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
  uint32_t t0 = millis();
  while (n < HTTP_MSG_LEN && (millis() - t0) < HTTP_TIMEOUT)
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
  int8_t pin[] = {A0, A2, A3, A4, A5, A6, 5, 13, 14};

  for (int8_t p : pin)
    pinMode(p, INPUT_PULLUP);

  analogRead(A1);
}

void pulse(int pin, uint32_t len) {
  digitalWrite(pin, HIGH);
  delay(len);
  digitalWrite(pin, LOW);
}

String &rc(command c, char i) {
  String *s = &c(i);
  if (s->length() == 0) {
    *s += i;
    *s += LF;
  }
  return *s;
}

String &readline(uint32_t timeout) {
  static bool init = false;
  static String s;

  if (!init) {
    s.reserve(75);
    init = true;
  }

  s = "";
  uint32_t t0 = millis();
  while ((millis() - t0) < timeout)
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
  if (!modem.testAT()) {
    disconnect(false);
    return connect();
  } else
    modem.init();

  if (!modem.waitForNetwork()) {
    modem.restart();
    if (!wait(MODEM_ON) || !modem.init() || !modem.waitForNetwork()) {
      disconnect();
      return connect();
    }
  }

  if (!modem.isGprsConnected())
    return gprs();
  return true;
}

bool resend() {
  while (w25q.read(msg, false)) {
    if (!post(msg))
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

bool valid(char c) { return (isPrintable(c) || c == '\r' || c == '\n'); }

bool verify() {
  if (!power && !connect())
    return false;
  if (!modem.isNetworkConnected() || !modem.isGprsConnected())
    if (!reconnect())
      return false;

  if (client.connect(HOST, PORT))
    return true;
  client.stop();
  delay(200L);
  if (!reconnect())
    return false;
  return client.connect(HOST, PORT);
}

bool wait(bool off, uint32_t timeout) {
  delay(2000L);
  uint32_t t0 = millis();
  while (millis() - t0 < timeout) {
    if (modem.testAT() ^ off)
      return true;
    delay(500L);
  }
  return false;
}

void setup() {
  pinMode(FET, OUTPUT);
  digitalWrite(FET, LOW);

  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, LOW);
  pinMode(SARA_PWR_ON, OUTPUT);
  digitalWrite(SARA_PWR_ON, LOW);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pullup();

  w25q.begin();
  if (digitalRead(PWR) == HIGH || battery() < 1) {
    ctrl();
    /* not reached */
  }
  w25q.sleep(true);

  location();

  enable();
  scan();
  disable();
  if (sid[0] == '\0')
    die(500L);

  msg.reserve(256);

  Wire.begin();
  SHTC3.begin();
  rtc.begin();

  if (!connect() || !config())
    die(1500L);

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
  msg = "";
  msg += rtc.getEpoch();
  msg += LF;

  float bat0 = battery();
  msg += '%';
  msg += SIGN(bat0);
  msg += bat0;

  bool psm = bat0 < BAT_LOW;

  SHTC3.readSample(true, psm);
  float st = SHTC3.getTemperature();
  float rh = SHTC3.getHumidity();
  msg += SIGN(st);
  msg += st;
  msg += SIGN(rh);
  msg += rh;
  msg += LF;

  enable();
  for (char *p = sid; *p; p++)
    msg += rc(measure, *p);
  disable();

  w25q.sleep(false);
  if (psm) {
    if (power)
      disconnect();
    w25q.append(msg);
  } else {
    if (!post(msg))
      w25q.append(msg);
    else
      resend();
  }
  w25q.sleep(true);

#if defined(MI_MINUTE)
  if (!psm)
#endif
    schedule();
  rtc.standbyMode();
}
