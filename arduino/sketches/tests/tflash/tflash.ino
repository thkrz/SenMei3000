#include <SPIMemory.h>

SPIFlash flash(SS1, &SPI1);
uint32_t addr;

void dump(String s) {

}

void blink(int n, uint32_t wait) {
  for (int i = 0; i < n; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(wait);
    digitalWrite(LED_BUILTIN, LOW);
    delay(wait);
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  if(!flash.begin())
    blink(10000, 200);


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
  blink(1, 1000);
}
