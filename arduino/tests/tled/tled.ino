
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  for (int i = 0; i < 255; i += 25) {
    for (int j = 0; j < 3; j++) {
      analogWrite(LED_BUILTIN, i);
      delay(1000);
      analogWrite(LED_BUILTIN, 0);
      delay(1000);
    }
  }
}
