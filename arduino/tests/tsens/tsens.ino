void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  int a1 = (int)(analogRead(A0)*0.0048875);
  int a2 = (int)(analogRead(A11)*0.0048875);

  for (int i = 0; i < a1; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(300);
    digitalWrite(LED_BUILTIN, LOW);
    delay(300);
  }

  for (int i = 0; i < a2; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(300);
    digitalWrite(LED_BUILTIN, LOW);
    delay(300);
  }

  delay(3000);
}
