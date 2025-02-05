#include <EEPROM.h>

#define NUM_CON 6

void setup() {
  Serial.begin(9600);
  delay(5000);

  Serial.println("Write flash");
  delay(5000);

  char c = 'A';
  for (int i = 0; i < NUM_CON; i++) {
    EEPROM.write(i, c+i);
  }
  Serial.println("done");
  delay(5000);

  Serial.println("Read flash");
  delay(5000);

  for (int i = 0; i < NUM_CON; i++) {
    char c = EEPROM.read(i);
    Serial.print(c);
  }
  Serial.println();
  Serial.println("done");
}

void loop() {
  delay(1000);
}
