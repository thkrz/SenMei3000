#include <SPIMemory.h>

SPIFlash flash(7);
uint32_t addr, paddr;


String load() {
  static String s;


  String q = "";
  s = "";
  for (uint32_t a = paddr; paddr < addr;) {
    if (flash.readStr(a, q))
      s += q;
    a += q.length();
  }
  return s;
}

void dump(String s) {
  flash.writeStr(addr, s);
  addr += s.length() + 1;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(5000);

  while (!flash.begin()) {
    Serial.println("No Chip selected");
    delay(10000);
  }

  addr = 0;
  paddr = 0;
  String s = "This is a test\n";
  dump(s);
  dump(s);

  String p = load();
  Serial.print(p);
}

void loop() {
  delay(100);
}
