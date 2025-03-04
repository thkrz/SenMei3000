#include <SDI12.h>
#include <SPI.h>

SDI12 socket(1, 4, 3);
//SDI12 socket(9);
char sid[63];

bool handshake(char i) {
  static char cmd[3] = "0!";

  Serial.print("Handshake [");
  Serial.print(i);
  Serial.print("]: ");

  cmd[0] = i;
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd);
    delay(30);
    if (socket.available()) {
      socket.clearBuffer();
      Serial.println("ACK");
      return true;
    }
  }
  socket.clearBuffer();
  Serial.println("NO");
  return false;
}

String& measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";
  static String s;

  st[0] = i;
  socket.sendCommand(st);
  delay(30);
  s = socket.readStringUntil('\n');
  uint8_t wait = s.substring(1, 4).toInt();

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
  int n = 0;
  for (char c = '0'; c <= '9'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  for (char c = 'a'; c <= 'z'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  sid[n] = '\0';
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  delay(5000);
  Serial.begin(9600);
  while (!Serial);

  socket.begin();
  delay(1000);
  scan();
}

void loop() {
  static char cmd[4] = "?I!";

  for (char *p = sid; *p; p++) {
    cmd[0] = *p;
    socket.sendCommand(cmd);
    delay(30);
    String s = socket.readStringUntil('\n');
    Serial.println(s);
    delay(1000);
    Serial.println(measure(*p));
    Serial.println();
  }
  delay(5000);
}
