#include <RTCZero.h>

#include "gsm.h"

#define CS  7

#define APN "iot.1nce.net"

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
  Serial.println("Begin SerialSARA");
  SerialSARA.begin(115200);
  Serial.println("Send powerpulse");
  powerpulse(150);
  delay(6000);
  Serial.println("Restart modem");
  modem.restart();
  Serial.print("Wait for network...");
  if (!modem.waitForNetwork()) {
    Serial.println("ERROR");
    return false;
  }
  Serial.println("OK");
  Serial.print("Connect to " APN "...");
  if (!modem.gprsConnect(APN)) {
    Serial.println("ERROR");
    return false;
  }
  Serial.println("OK");
  return true;
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
  digitalWrite(LED_BUILTIN, LOW);
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
}

void powerpulse(uint32_t len) {
  digitalWrite(SARA_PWR_ON, HIGH);
  delay(len);
  digitalWrite(SARA_PWR_ON, LOW);
}

bool reconnect() {
  //Serial.print("Connect to exmple.com...");
  //if (!client.connect("example.com", 80)) {
  //  Serial.println("ERROR");
  //  return false;
  //} else {
  //  Serial.println("OK");
  //  client.stop();
  //}
  Serial.print("Is network connected: ");
  if (!modem.isNetworkConnected()) {
    Serial.println("no");
    Serial.println("Restart modem");
    modem.restart();
    Serial.print("Wait for network...");
    if (!modem.waitForNetwork(90000L)) {
      Serial.println("ERROR");
      return false;
    }
    Serial.println("OK");
  } else
    Serial.println("yes");

  Serial.print("Is GPRS connected: ");
  if (!modem.isGprsConnected()) {
    Serial.println("no");
    Serial.print("Connect to " APN "...");
    if (!modem.gprsConnect(APN)) {
      Serial.println("ERROR");
      return false;
    }
    Serial.println("OK");
  }
  Serial.println("yes");
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
  delay(1000);

  if (!connect())
    die(1000);

  rtc.begin();
  Serial.print("Setting time...");
  settime();
  Serial.println("OK");

  rtc.setAlarmSeconds(0);
  rtc.enableAlarm(rtc.MATCH_SS);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println(rtc.getMinutes());
  reconnect();
  Serial.println("5 s");
  delay(5000);
  Serial.print("Connect to exmple.com...");
  if (!client.connect("example.com", 80, 5)) {
    Serial.println("ERROR");
  } else {
    Serial.println("OK");
    client.stop();
  }
  digitalWrite(LED_BUILTIN, LOW);
  rtc.standbyMode();
}
