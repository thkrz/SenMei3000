void disable() {
  digitalWrite(0, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(0, HIGH);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, OUTPUT);
}

void loop() {
  enable();
  delay(10000);
  disable();
  delay(10000);
}
