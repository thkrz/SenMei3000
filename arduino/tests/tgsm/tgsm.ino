#include <RTCZero.h>

#include "gsm.h"

#define MI_MINUTE 2
#define CS  7

static bool connect(bool fastboot = false);
static void die(uint32_t);
static void disable();
static void disconnect();
static void enable();
static void powerpulse(uint32_t);
static bool reconnect();
static void schedule();
static void settime();

RTCZero rtc;
TinyGsm modem(SerialSARA);
TinyGsmClient client(modem);

bool connect(bool fastboot) {
  SerialSARA.begin(115200);
  powerpulse(150);
  delay(6000);
  modem.restart();
  if (!modem.waitForNetwork())
    return false;
  return modem.gprsConnect(APN);
}

void die(uint32_t p) {
  for(;;) {
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
  if (modem.isGprsConnected())
    modem.gprsDisconnect();
  modem.poweroff();
  // powerpulse(1500);
  // power = false;
  // SerialSARA.end();
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(FET, HIGH);
  socket.begin();
  delay(600);
}

void powerpulse(uint32_t len) {
  digitalWrite(SARA_PWR_ON, HIGH);
  delay(len);
  digitalWrite(SARA_PWR_ON, LOW);
}

bool reconnect() {
  if (!modem.isNetworkConnected())
    if (!modem.waitForNetwork(90000L)) {
      modem.restart();
      if (!modem.waitForNetwork(90000L))
        return false;
    }
  if (!modem.isGprsConnected())
    return modem.gprsConnect(APN);
  return true;
  //if (modem.isNetworkConnected() && modem.isGprsConnected())
  //  return true;
  //if (power) {
  //  disconnect();
  //  delay(6000);
  //}
  //if (!connect())
  //  return false;
  //settime();
  //return true;
}

void schedule() {
  uint8_t m = (rtc.getMinutes() / MI_MINUTE + 1) * MI_MINUTE;
  rtc.setAlarmMinutes(m % 60);
}

void settime() {
  int Y, m, d, h, b, s;
  float z;

  if (modem.getNetworkTime(&Y, &m, &d, &h, &b, &s, &z)) {
    rtc.setDate(d, m, (Y-2000));
    rtc.setTime(h, b, s);
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, LOW);
  pinMode(SARA_PWR_ON, OUTPUT);
  digitalWrite(SARA_PWR_ON, LOW);

  Serial.begin(19200);
  while(!Serial);

  Serial.print("Connecting...");
  if (!connect()) {
    Serial.println("FAILED");
    die(1000);
  }
  Serial.println("OK");

  rtc.begin();
  Serial.print("Setting time...");
  settime();
  Serial.println("OK");

  //rtc.setAlarmSeconds(0);
  //rtc.enableAlarm(rtc.MATCH_MMSS);
  //schedule();
  //rtc.standbyMode();
}

void loop() {
  int db = modem.getSignalQuality();
  Serial.print(db);
  Serial.println(" db");
  if (!reconnect())
    Serial.println("Reconnect not possible");
  else
    Serial.println("Reconnected");
  delay(5000);
  //schedule();
  //rtc.standbyMode();
}
