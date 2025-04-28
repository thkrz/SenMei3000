#define FET_PIN 0
#define MX 1
#define RX 4
#define TX 3


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FET_PIN, OUTPUT);
  digitalWrite(FET_PIN, LOW);

  pinMode(MX, OUTPUT);
  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);

  // WRITE
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(MX, LOW);

  // READ
  //digitalWrite(LED_BUILTIN, HIGH);
  //digitalWrite(MUX_PIN, HIGH);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(TX, HIGH);
  delay(3000);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(TX, LOW);
  delay(3000);
}
