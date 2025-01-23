#include <SHTC3.h>
#include <Wire.h>

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  while (!Serial);

  Wire.begin();
  if(!SHTC3.begin()) {
    Serial.println("ERROR");
    for (;;)
      delay(100);
  }
}

void loop() {
  SHTC3.readSample();
  String s = "";
  s += String(SHTC3.getTemperature()) + "\r\n";
  s += String(SHTC3.getHumidity()) + "\r\n";

  Serial.print(s);
  delay(5000);
}
