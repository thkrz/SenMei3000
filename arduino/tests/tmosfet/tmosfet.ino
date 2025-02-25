#define FET 0

void disable() {
  digitalWrite(FET, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(FET, HIGH);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FET, OUTPUT);
  disable();
}

void loop() {
  delay(10000);
  enable();
  delay(10000);
  disable();
}
