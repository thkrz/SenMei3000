#include <SDI12.h>
#include <SPI.h>

SDI12 socket(3);
char sid[63];

void enable() {
  Serial.println("Socket enabled");
  socket.begin();
  delay(500);
}

bool handshake(char i) {
  static char cmd[3] = "0!";

  Serial.print("Handshake [");
  Serial.print(i);
  Serial.print("]: ");

  cmd[0] = i;
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd);
    socket.clearBuffer();
    delay(30);
    if (socket.available()) {
      Serial.println("ACK");
      return true;
    }
  }
  socket.clearBuffer();
  Serial.println("no response");
  return false;
}

void scan() {
  int n = 0;
  for (char c = '0'; c <= '9'; c++) {
    if (handshake(c))
      sid[n++] = c;
    delay(500);
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    if (handshake(c))
      sid[n++] = c;
    delay(500);
  }
  for (char c = 'a'; c <= 'z'; c++) {
    if (handshake(c))
      sid[n++] = c;
    delay(500);
  }
  sid[n] = '\0';
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  delay(5000);
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Scanning...");
  enable();
  scan();

  Serial.println("done");
}

void loop() {
  //Serial.println("-- Send command --");
  //for (char *p = sid; *p; p++) {
  //  Serial.println(ident(*p));
  //  delay(100);
  //  Serial.println(measure(*p));
  //  delay(100);
  //}
  //delay(10000);
  delay(100);
}
