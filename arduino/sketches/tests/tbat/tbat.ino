float battery() {
  int p = analogRead(A1);
  return (float)p * 0.014956;  // R1 = 1.2M; R2 = 330k
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  int V = (int)battery();
  for (int i = 0; i < V; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
  delay(5000);
}
