#include <SPI.h>
#include <SPIMemory.h>

#define BLK_SZ 4096
#define CS 7


SPIFlash flash(CS);
char cache[BLK_SZ];
int len = 0;

void del(int j) {
  for (; j > 0; j--)
    cache[len--] = 0;
  sync();
}

bool dump(String s) {
  int n = s.length() + 1;
  if (len + n > BLK_SZ)
    return false;
  s.toCharArray(&cache[len], n);
  len += n;
  sync();
  return true;
}

void init() {
  flash.readCharArray(0, cache, BLK_SZ);
  len = BLK_SZ;
  while (cache[len] == 0)
    len--;
}

void sync() {
  flash.eraseSector(0);
  flash.writeCharArray(0, cache, BLK_SZ);
}

int load(String &s) {
  static char rev[256];
  int j;

  int i = len;
  while (i > 0 && cache[i] == 0)
    i--;
  for (j = 0; j < 255 && i > 0 && cache[i] != 0; j++) {
    rev[j] = cache[i--];
  }
  rev[j] = '\0';
  s = String(rev);
  return j;
}

void setup() {
  delay(5000);

  Serial.begin(9600);
  while (!Serial);

  flash.begin();
  Serial.print("ERASE CHIP...");
  flash.eraseChip();
  Serial.println("DONE");

  init();
  String s = "Hello World!\r\n";
  dump(s);

  s = "Uhhhhh\r\n";
  dump(s);

  String p;
  while (load(p) > 0)
    Serial.println(p);
}

void loop() {
  delay(500);
}
