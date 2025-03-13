#define RAIN 7

uint32_t mm;

void isr() {
  mm++;
}

void setup() {
  pinMode(7, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(7), isr, FALLING);
  Serial.begin(19200);
  while (!Serial);
  delay(3000);
}

void loop() {
  Serial.println(mm);
  delay(5000);
}
