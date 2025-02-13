#define MUX_PIN 1


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MUX_PIN, OUTPUT);

  // WRITE
  //digitalWrite(MUX_PIN, LOW);
  //pinMode(2, OUTPUT);

  // READ
  digitalWrite(MUX_PIN, HIGH);
  pinMode(2, INPUT);
}

void loop() {
  delay(100);
}
