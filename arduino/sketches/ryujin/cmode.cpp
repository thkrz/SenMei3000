#include <Arduino.h>

bool format = false;

void begin() {
  Serial.begin(19200);
  while (!Serial);
}

void run() {
  for (;;) {
    if (Serial.available()) {
      char c = Serial.read();
      if (format) {
        Serial.println();
        if (c == 'y') {
          Serial.print(F("ERASE..."));
          erase();
          Serial.println(F("DONE"));
        }
        format = false;
      } else
          switch(c) {
          case 'd':
            String s;
            uint32_t len = LEN;
            while(load(s)) {
              Serial.print(s);
              discard();
            }
            LEN = len;
            Serial.print("#");
            break;
          case 'f':
            format = true;
            Serial.print(F("Perform chip erase? [y/N]: "));
            break;
          case 't':
            Serial.println(F("Run tests"));
            break;
          case '?':
            Serial.println(F("d: dump flash chip"));
            Serial.println(F("f: format flash chip"));
            Serial.println(F("?: show this message"));
            break;
          }
    delay(100);
  }
}
