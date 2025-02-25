#include <SPI.h>
#include <SPIMemory.h>

#define CAP 1024
#define LEN (addr[0])
#define BSZ (sizeof(uint32_t))

#define CS 7


SPIFlash flash(CS);
uint32_t addr[CAP];

void dir() {
  for (int i = 0; i < CAP; i++)
    addr[i] = flash.readULong(i*BSZ);
}

void discard() {
  if (LEN > 0)
    LEN--;
}

bool push(String s) {
  if (LEN < CAP) {
    uint32_t a = flash.getAddress(flash.sizeofStr(s));
    if (a == 0)
      return false;
    addr[LEN+1] = a;
    if (flash.writeStr(a, s)) {
      LEN++;
      sync();
      return true;
    }
  }
  return false;
}

void sync() {
  while (!flash.eraseSector(0));
  for (int i = 0; i < CAP; i++)
    flash.writeULong(i*BSZ, addr[i]);
}

bool pop(String &s) {
  if (LEN < 1)
    return false;
  uint32_t a = addr[LEN];
  return flash.readStr(a, s);
}

void setup() {
  delay(5000);

  Serial.begin(9600);
  while (!Serial);

  flash.begin();
  //Serial.print("ERASE CHIP...");
  //flash.eraseChip();
  //Serial.println("DONE");

  //sync();

  //Serial.println(test());
}

void loop() {
  delay(500);
}
