#include <SPI.h>
#include <SPIMemory.h>

#define CAP 1024

#define CS  7

#define BSZ 4 // (sizeof(uint32_t))
#define LF "\r\n"
#define LEN (addr[0])

static void dir();
static void discard();
static bool dump(String&);
static void erase();
static void insert(int n = 3);
static bool load(String&);
static void reprint();
static void sync();

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

bool dump(String &s) {
  if (LEN < CAP-1) {
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

void erase() {
  flash.eraseChip();
  for (int i = 0; i < CAP; i++) {
    flash.writeULong(i*BSZ, 0);
    addr[i] = 0;
  }
}

void insert(int n) {
  static int i = 0;

  for (int j = 0; j < n; j++) {
    String s = "Teststring";
    s += i++;
    dump(s);
  }
}

bool load(String &s) {
  if (LEN < 1)
    return false;
  return flash.readStr(addr[LEN], s);
}

void reprint() {
  String s;
  while (load(s)) {
    Serial.print(s);
    discard();
    sync();
  }
}

void sync() {
  while (!flash.eraseSector(0));
  for (int i = 0; i < CAP; i++)
    flash.writeULong(i*BSZ, addr[i]);
}

void setup() {
  Serial.begin(19200);
  while (!Serial);
  delay(3000);

  flash.begin();
  dir();
  insert();
  flash.powerDown();
}

void loop() {
  flash.powerUp();
  if (LEN > 0)
    reprint();
  flash.powerDown();
  Serial.println("---");
  delay(15000);
}
