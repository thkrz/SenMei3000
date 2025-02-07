void debug() {
  Serial.begin(9600);
  while (!Serial);

  for (;;) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '!') {
        String s = load();
        Serial.print(s + "#");
      }
    }
    delay(100);
  }
}

String gps() {
  //Serial1.begin(9600);
  //while (!Serial1);
  // send GPS
  //Serial1.close();
}
