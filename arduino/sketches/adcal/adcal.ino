#include <SDI12.h>

SDI12 socket(9);
char sid[63];

void disable() {
  socket.end();
  digitalWrite(LED_BUILTIN, LOW);
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  socket.begin();
  delay(500);
}

bool handshake(char i) {
  static char cmd[3] = "0!";

  cmd[0] = i;
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd);
    socket.clearBuffer();
    delay(30);
    if (socket.available())
      return true;
  }
  socket.clearBuffer();
  return false;
}

String ident(char i) {
  static char cmd[4] = "aI!";
  static String s;

  cmd[0] = i;
  socket.sendCommand(cmd);
  delay(300);
  s = socket.readStringUntil('\n');
  return s;
}

void idle() {
  for(;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}

String measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";
  static String s;

  Serial.println();
  socket.clearBuffer();
  delay(10);
  st[0] = i;
  socket.sendCommand(st);
  delay(500);
  s = socket.readStringUntil('\n');
  uint8_t wait = s.substring(1, 4).toInt();
  s.trim();
  Serial.println(st);
  Serial.print("RESP: ");
  Serial.print(s);
  Serial.print(", WAIT: ");
  Serial.println(wait);

  for (int j = 0; j < wait; j++) {
    if (socket.available()) {
      socket.clearBuffer();
      break;
    }
    delay(1000);
  }

  rd[0] = i;
  socket.sendCommand(rd);
  delay(30);
  s = socket.readStringUntil('\n');
  return s;
}

void scan() {
  sid[0] = '1';
  sid[1] = '\0';
  return;
  int n = 0;
  for (char c = '0'; c <= '9'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  for (char c = 'a'; c <= 'z'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  sid[n] = '\0';
}

float readVoltage(int8_t pin) {
  analogRead(pin);
  int x = analogRead(pin);
  return x * (5.0/1023.0);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial);
  delay(5000);

  enable();
  scan();
  // disable();

  Serial.print("ADDR: ");
  Serial.println(sid);

  if (sid[0] == '\0')
    idle();

  Serial.println("--");
}

void loop() {
  for (char *p = sid; *p; p++) {
    Serial.print("I: ");
    String I = ident(*p);
    I.trim();
    Serial.print(I);
    Serial.print(", M: ");
    String M = measure(*p);
    M.trim();
    Serial.println(M);
  }

  float temp = readVoltage(A0);
  float vwc = readVoltage(A1);
  Serial.print("TEMP: ");
  Serial.print(temp);
  Serial.print(", VWC: ");
  Serial.println(vwc);


  Serial.println();
  delay(15000);
}
