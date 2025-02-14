#include <SDI12.h>

#define ADDR 'a'
#define CMD_LEN 12

SDI12 socket(9);
char buf[CMD_LEN];
int len = 0;

bool isvalid(char a) {
  return ((a >= '0' && a <= '9') ||
    (a >= 'a' && a <= 'z') ||
    (a >= 'A' && a <= 'Z'));
}

void rc() {
  char addr = buf[0];
  Serial.print(len);
  Serial.print(": ");
  Serial.println(addr);
  if (addr != ADDR)
    return;
  String r = "";
  bool m = false;
  if (len > 1)
    switch (buf[1]) {
    case 'I':
      r = "13KREUZER";
      break;
    case 'M':
      m = true;
      r = "0022";
      break;
    case 'D':
      r = "+20.0-4.1";
      break;
    case 'A':
      Serial.println("address change");
      break;
    }

  String s = String(addr) + r + "\r\n";
  socket.sendResponse(s);
  Serial.println(s);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial);

  socket.begin();
  delay(500);
  socket.forceListen();
}

void loop() {
  if (socket.available()) {
    char c = socket.read();
    if (c == '!') {
      buf[len] = '\0';
      Serial.print("Command recieved, len ");
      Serial.print(len);
      Serial.print(": ");
      for (int i = 0; i < len; i++) {
        Serial.print((int)buf[i]);
        Serial.print(",");
      }
      Serial.println();
      socket.clearBuffer();
      socket.forceHold();
      if (len > 0) {
        rc();
        len = 0;
      }
      socket.forceListen();
    } else if (len < CMD_LEN)
      buf[len++] = c;
  }
}
