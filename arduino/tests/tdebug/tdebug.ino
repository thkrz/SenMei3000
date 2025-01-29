void debug() {
  String s = "2025-01-01T12:15\r\n12.5\r\n23.1\r\n\r\n2025-01-01T13:15\r\n12.4\r\n23.1\r\n#";
  for (;;) {
    if(Serial.available()) {
      char c = Serial.read();
      if (c == '!')
        Serial.print(s);
    }
    delay(100);
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  debug();
}

void loop() {
}
