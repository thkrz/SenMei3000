#include <RTCZero.h>
#include "gsm.h"

#define APN "iot.1nce.net"

static bool connect();
static void disconnect();
static bool gprs();
static bool ping();
static void pulse(int, uint32_t);
static bool reconnect();
static void settime();
static bool wait(uint32_t timeout = MODEM_TIMEOUT);

RTCZero rtc;
TinyGsm modem(SerialSARA);
TinyGsmClient client(modem);
bool power;

bool connect() {
  Serial.println("Bringing SerialSARA up");
  SerialSARA.begin(115200);
  Serial.println("Power pulse");
  pulse(SARA_PWR_ON, 200);
  power = true;
  if (!wait() || !modem.init() || !modem.waitForNetwork())
    return false;
  Serial.println("Connecting GPRS");
  return gprs();
}

void disconnect() {
  Serial.println("Disconnect");
  client.stop();
  if (modem.isGprsConnected())
    modem.gprsDisconnect();
  if (modem.poweroff()) {
    delay(5000);
    power = false;
    for (uint8_t j = 0; j < 3; j++) {
      if (modem.testAT()) {
        power = true;
        break;
      }
      delay(200);
    }
  }
  if (!power)
    SerialSARA.end();
}

bool gprs() {
  for (uint8_t j = 0; j < 5; j++) {
    if (modem.gprsConnect(APN)) {
      settime();
      return true;
    }
    delay(1000);
  }
  return false;
}

bool ping() {
  if (!modem.isNetworkConnected() || !client.connect("8.8.8.8", 53))
    if (!reconnect() || !client.connect("8.8.8.8", 53))
      return false;
  client.stop();
  return true;
}

void pulse(int pin, uint32_t len) {
  digitalWrite(pin, HIGH);
  delay(len);
  digitalWrite(pin, LOW);
}

bool reconnect() {
  Serial.println("Reconnect...");
  if (!power)
    return connect();

  if (!modem.testAT()) {
    Serial.println("RESET");
    pulse(SARA_RESETN, 150);
    if (!wait() || !modem.init()) {
      disconnect();
      return connect();
    }
  } else
    modem.init();

  if (!modem.waitForNetwork()) {
    Serial.println("RESTART");
    modem.restart();
    if (!wait() || !modem.init() || !modem.waitForNetwork()) {
      disconnect();
      return connect();
    }
  }

  client.stop();
  if (modem.isGprsConnected()) {
    modem.gprsDisconnect();
    delay(200);
  }
  return gprs();
}

void settime() {
  int Y, m, d, h, b, s;
  float z;

  if (modem.getNetworkTime(&Y, &m, &d, &h, &b, &s, &z)) {
    rtc.setDate(d, m, (Y - 2000));
    rtc.setTime(h, b, s);
  }
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
  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, LOW);
  pinMode(SARA_PWR_ON, OUTPUT);
  digitalWrite(SARA_PWR_ON, LOW);

  rtc.begin();

  Serial.begin(9600);
  while (!Serial);

  if (!connect())
    Serial.println("Connection error");
}

void loop() {
  Serial.print("PING: ");
  Serial.println(ping());
  delay(15000L);
}
