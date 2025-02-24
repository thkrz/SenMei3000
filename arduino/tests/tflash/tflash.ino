#include <SPI.h>
#include <SPIMemory.h>

#define HSZ 512
#define CS 7


SPIFlash flash(CS);

bool dump(String s) {
  uint32_t addr = flash.getAddress(flash.sizeofStr(s));

  flash.eraseSector(i*4);
  return flash.writeULong(i++*4, addr) && flash.writeStr(addr, s);
}

bool load(String &s) {
  static int i = 0;

  if (i >= 512)
    return false;
  uint32_t addr = flash.readULong(i*sizeof(uint32_t));
  if (addr > 512*4) {
    i++;
    return flash.readStr(addr, s);
  }
  return false;
}

void setup() {
  delay(5000);

  Serial.begin(9600);
  while (!Serial);

  flash.begin();
  Serial.print("ERASE CHIP...");
  flash.eraseChip();
  Serial.println("DONE");
  Serial.print("WRITE HEADER...");
  for (int i = 0; i < HSZ; i++) {
    if (!flash.writeULong(i*4, 0))
      Serial.println("ERROR");
  }
  Serial.println("DONE");
}

void loop() {
  delay(500);
}
