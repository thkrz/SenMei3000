#include <SPI.h>
#include <SPIMemory.h>

#define HSZ 2
#define CS 7


SPIFlash flash(CS);

int dump(String s) {
  int i;

  uint16_t k = flash.readWord(0);
  size_t n = s.length();
  for (i = 0; i < n; i++)
    if (!flash.writeChar(i+k+HSZ, s.charAt(i)))
      break;
  flash.eraseSector(0);
  flash.writeWord(0, k+i);
  return i;
}

String load() {
  uint16_t k = flash.readWord(0);
  Serial.println(k);
  char buf[k+1];
  for (int i = 0; i < k; i++)
    buf[i] = flash.readChar(i);
  buf[k] = '\0';
  Serial.println((char)buf[2]);
  return String(buf);
}

void setup() {
  delay(5000);

  Serial.begin(9600);
  while (!Serial);

  flash.begin();
  //Serial.print("ERASE CHIP...");
  //flash.eraseChip();
  //flash.writeWord(0, 0);
  //Serial.println("DONE");
  //erase();
  Serial.println(flash.getAddress(12));
  //int n = dump("Hello World!");
  //Serial.println(n);
  String s = load();
  //Serial.println(s);
}

void loop() {
  delay(500);
}
