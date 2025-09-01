#include <RTCZero.h>

static void ctrl();

RTCZero rtc;

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
          break;
        case 'f':
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

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(SARA_RESETN, OUTPUT);
  digitalWrite(SARA_RESETN, LOW);
  pinMode(SARA_PWR_ON, OUTPUT);
  digitalWrite(SARA_PWR_ON, LOW);

  rtc.begin();

  ctrl();
  /* not reached */
}

void loop() {
}
