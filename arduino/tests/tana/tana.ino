#include <SDI12.h>

#define CMD_LEN 8
#define BUS_PIN 9

SDI12 socket(BUS_PIN);
char buf[CMD_LEN];
int len = 0;

void setup() {
  Serial.begin(19200);
  while (!Serial);

  socket.begin();
  socket.forceListen();
}

void loop() {
  if (socket.available()) {
    char c = socket.read();
    if (c == '!') {
      buf[len] = '\0';
      socket.clearBuffer();
      socket.forceHold();
      if (len > 0) {
        Serial.print("COMMAND RECEIVED: ");
        Serial.println(buf);
        len = 0;
      }
      socket.forceListen();
    } else if (c > 0 && len < CMD_LEN)
      buf[len++] = c;
  }
}
