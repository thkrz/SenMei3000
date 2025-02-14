#define MUX_PIN 1


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MUX_PIN, OUTPUT);
  pinMode(2, INPUT);
  pinMode(3, OUTPUT);

  // WRITE
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(MUX_PIN, LOW);

  // READ
  //digitalWrite(LED_BUILTIN, HIGH);
  //digitalWrite(MUX_PIN, HIGH);
}

void loop() {
  digitalWrite(3, HIGH);
  delay(500);
  digitalWrite(3, LOW);
  delay(500);

}
