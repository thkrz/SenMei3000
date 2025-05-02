#include <MKRNB.h>
#include <RTCZero.h>
#include <SHTC3.h>
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
static void die();
static void disable();
static char *prnt2(uint8_t);
static bool post(String&);
static void pullup();
static void schedule();
static void verify();

GPRS gprs;
NBClient client;
NB nbAccess;
RTCZero rtc;
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
  digitalWrite(FET, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

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
    if (n == HTTP_MSG_LEN && HTTP_MSG_OK(buf))
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

void schedule() {
#if defined(MI_MINUTE)
  uint8_t m = (rtc.getMinutes() / MI_MINUTE + 1) * MI_MINUTE;
  rtc.setAlarmMinutes(m % 60);
#elif defined(MI_HOUR)
  uint8_t m = (rtc.getHours() / MI_HOUR + 1) * MI_HOUR;
  rtc.setAlarmHours(m % 24);
#endif
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

  //if (battery() == 0)
  //  ctrl();
    /* not reached */

  connect();

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
  digitalWrite(LED_BUILTIN, HIGH);
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

  SHTC3.readSample(true, false);
  float st = SHTC3.getTemperature();
  float rh = SHTC3.getHumidity();
  q += SIGN(st);
  q += st;
  q += SIGN(rh);
  q += rh;
  q += LF;

  verify();
  post(q);

  schedule();
  digitalWrite(LED_BUILTIN, LOW);
  rtc.standbyMode();
}
