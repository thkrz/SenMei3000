#include <SDI12.h>
#include <SPI.h>

#define DATA_PIN 3

SDI12 socket(DATA_PIN);
char sid[63];

bool handshake(char i) {
  static char cmd[3] = "0!";

  cmd[0] = i;
  Serial.println(cmd);
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd);
    socket.clearBuffer();
    delay(30);
    if (socket.available()) {
      return true;
    }
  }
  socket.clearBuffer();
  return false;
}

void info(char i) {
  static char st[4] = "aI!";
  socket.clearBuffer();
  st[0] = i;
  socket.sendCommand(st);
  delay(380);
  String s = socket.readStringUntil('\n');
  Serial.println(s);
}

String measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";
  static String s;

  socket.clearBuffer();
  st[0] = i;
  socket.sendCommand(st);
  delay(380);
  s = socket.readStringUntil('\n');
  // s.trim();
  uint8_t wait = s.substring(1, 4).toInt();
  // int num = r.substring(4).toInt();

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
  Serial.println("Search...");
  socket.begin();
  delay(500);
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
  socket.end();
  sid[n] = '\0';
  Serial.print("Devices: ");
  Serial.println(sid);
}

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  pinMode(DATA_PIN, INPUT);
  scan();
}

void loop() {
    delay(5000);

    String s = "";

    socket.begin();
    delay(500);
    for (char *p = sid; *p; p++) {
      s += measure(*p);
    }
    socket.end();

    Serial.print(s);
}

