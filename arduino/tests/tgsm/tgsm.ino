#include <RTCZero.h>

#include "config.h"
#include "gsm.h"

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

static bool connect(bool fastboot = false);
static void disconnect();
static bool post(String&);
static void powerpulse(uint32_t);
static void sarainit();
static void schedule();
static void settime();

RTCZero rtc;
TinyGsm modem(SerialSARA);
TinyGsmClient client(modem);

bool connect(bool fastboot) {
  Serial.print("Set baud rate...");
  SerialSARA.begin(115200);
  Serial.println("done");
  Serial.print("Send power pulse 150...");
  powerpulse(150);
  Serial.println("done");
  delay(6000);
  Serial.print("Restart modem...");
  if (fastboot)
    modem.init();
  else
    modem.restart();
  Serial.println("done");
  Serial.print("Wait for network...");
  if (!modem.waitForNetwork())
    return false;
  Serial.println("done");
  Serial.print("Connect to "APN"...");
  return modem.gprsConnect(APN);
}

void disconnect() {
  Serial.print("Disconnect...");
  modem.gprsDisconnect();
  Serial.println("done");
  Serial.print("Power off...");
  modem.poweroff();
  Serial.println("done");
  Serial.print("Send power pulse 1500...");
  powerpulse(1500);
  Serial.println("done");
  Serial.print("End serial...");
  SerialSARA.end();
  Serial.println("done");
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

  uint32_t timeout = millis();
  while (!client.available())
    if ((millis() - timeout) > HTTP_TIMEOUT) {
      client.stop();
      return false;
    }

  n = 0;
  char buf[HTTP_MSG_LEN];
  while (client.available()) {
    char c = client.read();
    if (n < HTTP_MSG_LEN)
      buf[n++] = c;
  }
  client.stop();
  return HTTP_OK(buf, n);
}

void powerpulse(uint32_t duration) {
  digitalWrite(SARA_PWR_ON, HIGH);
  delay(duration);
  digitalWrite(SARA_PWR_ON, LOW);
}

void sarainit() {
  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, LOW);
  pinMode(SARA_PWR_ON, OUTPUT);
  digitalWrite(SARA_PWR_ON, LOW);
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
#if defined(COMPILE_TIME)
  else
    rtc.setEpoch(COMPILE_TIME);
#endif
}

void setup() {
  Serial.begin(11900);
  while (!Serial);
  delay(5000);

  sarainit();
  Serial.println("BEGIN");

  if (connect())
    Serial.println("-- Connected --");
  else {
    Serial.println("ERROR");
    for (;;)
      delay(100);
  }

  rtc.begin();
  Serial.println("Set time...");
  settime();
  Serial.println("done");
  disconnect();
  Serial.println("-- Disconnected --");

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
  if (connect())
    Serial.println("OK");
  else {
    Serial.println("ERROR");
    for (;;)
      delay(100);
  }


  String s = "TEST 123";
  Serial.print("POST HTTP...");
  if (!post(s))
    Serial.println("ERROR");
  else
    Serial.println("OK");

  disconnect();
  Serial.println("-- Disconnected --");

  Serial.println("Go to sleep");
  schedule();
  rtc.standbyMode();
}
