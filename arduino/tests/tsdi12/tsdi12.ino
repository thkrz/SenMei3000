#include <SDI12.h>
#include <SPI.h>

#define FET 0 /* LED_BUILTIN */
#define MX  1
#define RX  4
#define TX  3

#define LF "\r\n"
#define WAKE_DELAY 0
#define SDI_TIMEOUT 3000

void disable();
void enable();
String& readline(uint32_t timeout = SDI_TIMEOUT);

SDI12 socket(MX, RX, TX);
char cmd[128];
int len = 0;
char sid[63];

void disable() {
  digitalWrite(FET, LOW);
}

void enable() {
  digitalWrite(FET, HIGH);
  delay(600);
}

//int nval(String &s) {
//  int n = 0;
//  for (char *p = s.c_str(); *p; p++)
//    if (*p == '+' || *p == '-')
//      n++;
//  return n;
//}
bool handshake(char i) {
  static char cmd[3] = "a!";

  Serial.print("HANDSHAKE [");
  Serial.print(i);
  Serial.print("]: ");
  cmd[0] = i;
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd, WAKE_DELAY);
    String s = readline(250);
    if (s.charAt(0) == i) {
      Serial.println("ACK");
      return true;
    }
  }
  socket.clearBuffer();
  Serial.println("NA");
  return false;
}

String& measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";

  st[0] = i;
  socket.sendCommand(st, WAKE_DELAY);
  String s = readline();
  Serial.print(s);
  uint8_t wait = s.substring(1, 4).toInt();
  //uint8_t num = s.charAt(4) - '0';

  for (int j = 0; j < wait; j++) {
    if (socket.available()) {
      socket.clearBuffer();
      break;
    }
    delay(1000);
  }

  rd[0] = i;
  socket.sendCommand(rd, WAKE_DELAY);
  return readline();
}

String& readline(uint32_t timeout) {
  static String s;

  s = "";
  uint32_t st = millis();
  while ((millis() - st) < SDI_TIMEOUT) {
    if (socket.available()) {
      char c = socket.read();
      s += c;
      if (c == '\n')
        break;
    } else
      delay(10);
  }
  socket.clearBuffer();
  return s;
}

void scan() {
  int n = 0;
  for (char c = '0'; c <= '3'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  for (char c = 'a'; c <= 'd'; c++) {
    if (handshake(c))
      sid[n++] = c;
  }
  sid[n] = '\0';
}

void setup() {
  pinMode(FET, OUTPUT);
  disable();

  Serial.begin(19200);
  while (!Serial);
  socket.begin();
  delay(500);
}

void loop() {
  String s;

  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
    case '\r':
      Serial.println();
      if (len > 1 && cmd[len-1] == '!') {
        cmd[len] = '\0';
        enable();
        if (cmd[1] == 'M')
          s = measure(cmd[0]);
        else if (strncmp(cmd, "scan", 4) == 0)
          scan();
        else {
          socket.sendCommand(cmd, WAKE_DELAY);
          s = readline();
        }
        disable();
        if (s == "")
          Serial.println(F("ERROR"));
        else
          Serial.print(s);
      } else
          Serial.println(F("UNKNOWN COMMAND"));
      len = 0;
      break;
    default:
      Serial.write(c);
      if (len < 127)
        cmd[len++] = c;
      break;
    }
  }
}
