float battery() {
  int p = analogRead(A1);
  return (float)p * 0.014956;  // R1 = 1.2M; R2 = 330k
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
}

void loop() {
  Serial.print("BAT: ");
  Serial.print(battery());
  Serial.println(" V");
  delay(5000);
}
