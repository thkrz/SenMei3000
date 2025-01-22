#include <SPIMemory.h>

SPIFlash flash;
uint32_t addr;

void dump(String s) {

}

void blink(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  blink(2);
  delay(1000);
  flash.begin();
  blink(4);
  delay(1000);

  addr = 0;
  String s = "Hello World!\n";
  if(flash.writeStr(addr, s))
    blink(5);

  //addr = flash.sizeofStr(s);
  //blink(addr);
  //delay(1000);
  //addr = flash.getAddress(addr);
  //blink(addr);
  //delay(1000);


  //String q = "What's up?\n";
  //flash.writeStr(addr, q);
  //Serial.print(q);
  //Serial.println(" written");

  //String r = "";
  //if (flash.readStr(0, r)) {
  //  Serial.println(r);
  //} else {
  //  Serial.println("READ ERROR");
  //}

}

void loop() {
}
