#include "gsm.h"

#define APN "iot.1nce.net"
// #define APN "m2m.vodafone.de"

bool connect();
void disconnect(bool alive = true);
bool gprs();
void pulse(int, uint32_t);
bool reconnect();
bool verify();
bool wait(bool);

TinyGsm modem(SerialSARA);
TinyGsmClient client(modem);

bool connect() {
  if (!wait(MODEM_ON) || !modem.init())
    return false;
  Serial.print("MODEM: ");
  String s = modem.getModemInfo();
  Serial.println(s);
  Serial.print("Wait for network...");
  if (!modem.waitForNetwork(90000UL)) {
    Serial.println("ERROR");
    return false;
  }
  Serial.println("OK");
  return true;
}

bool gprs() {
  uint8_t pause[] = { 1, 3, 5 };
  Serial.print("Connect GPRS...");
  for (uint8_t p : pause) {
    if (modem.gprsConnect(APN)) {
      Serial.println("OK");
      return true;
    }
    delay(p * 1000UL);
  }
  Serial.println("ERROR");
  return false;
}

void pulse(int pin, uint32_t len) {
  digitalWrite(pin, HIGH);
  delay(len);
  digitalWrite(pin, LOW);
}

bool verify() {
  if (!gprs())
    return false;

  Serial.print("Ping...");
  if (client.connect("8.8.8.8", 53)) {
    Serial.println("OK");
    return true;
  }
  Serial.println("ERROR");
  return false;
}

bool wait(bool off) {
  Serial.print("Wait for modem to come ");
  Serial.print(off ? "offline..." : "online...");
  delay(2000UL);
  uint32_t t0 = millis();
  while (millis() - t0 < MODEM_TIMEOUT) {
    if (modem.testAT() ^ off) {
      Serial.println("OK");
      return true;
    }
    delay(500UL);
  }
  Serial.println("ERROR");
  return false;
}

void setup() {
  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, LOW);
  pinMode(SARA_PWR_ON, OUTPUT);
  digitalWrite(SARA_PWR_ON, LOW);

  Serial.begin(9600);
  while (!Serial);

  Serial.print("SerialSARA...");
  SerialSARA.begin(115200);
  Serial.println("OK");
  Serial.print("Power pulse...");
  pulse(SARA_PWR_ON, 1500UL);
  Serial.println("OK");

  while (!connect()) {
    Serial.println("Retry in 3s");
    Serial.println();
    delay(3000UL);
  }
}

void loop() {
  Serial.print("dB: ");
  int sq = modem.getSignalQuality();
  Serial.println(sq);
  delay(5000UL);
}
