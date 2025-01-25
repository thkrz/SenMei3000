#include <SPIMemory.h>

SPIFlash flash(7);
uint32_t addr;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  while (!flash.begin()) {
    Serial.println("No Chip selected");
    delay(10000);
  }
}

void loop() {
}
