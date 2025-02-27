#include <MKRNB.h>
#include <RTCZero.h>
#include <SDI12.h>
#include <SPI.h>
#include <Wire.h>
#include <SHTC3.h>

#include "config.h"
#include "global.h"

#define MSG_LEN 12
//#define FET 0
#define MX  1
#define RX  4
#define TX  3

#define LF "\r\n"
#define SIGN(x) (((x)>=0?'+':'\0')+String(x))
#define PRNT2(x) (((x)<10?'0':'\0')+String(x))

GPRS gprs;
NBClient client;
NB nbAccess;
RTCZero rtc;
SDI12 socket(MX, RX, TX);
char sid[63];

/* ***************** SDI12 *********** */
bool handshake(char i) {
  static char cmd[3] = "a!";

  cmd[0] = i;
  for (int j = 0; j < 3; j++) {
    socket.sendCommand(cmd);
    delay(30);
    if (socket.available()) {
      socket.clearBuffer();
      return true;
    }
  }
  socket.clearBuffer();
  return false;
}

String ident(char i) {
  static char cmd[4] = "aI!";
  static String s;

  cmd[0] = i;
  socket.sendCommand(cmd);
  delay(30);
  s = socket.readStringUntil('\n');
  return s;
}

String measure(char i) {
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

bool update() {
  String s = "UPDATE\r\n";
  for (char *p = sid; *p; p++)
    s += ident(*p);
  return post(s);
}

/* ***************** GSM *********** */
void connect() {
  bool connected = false;
  while (!connected) {
    if ((nbAccess.begin() == NB_READY)
        && (gprs.attachGPRS() == GPRS_READY)) {
      connected = true;
    } else {
      delay(1000);
    }
  }
}

bool post(String &s) {
  int n = s.length();
  if (n == 0)
    return false;
  bool ok = false;
  if (client.connect(HOST, PORT)) {
    client.println(F("POST "PATH"/"STAT_CTRL_ID" HTTP/1.1"));
    client.println(F("Host: "HOST));
    client.println(F("Connection: close"));
    client.println(F("Content-Type: text/plain"));
    client.print(F("Content-Length: "));
    client.println(n);
    client.println();
    client.print(s);

    char buf[MSG_LEN];
    for (int i = 0; !ok && i < MSG_LEN; i++) {
      while (!client.available())
        delay(10);
      buf[i] = client.read();
    }
    ok = strncmp("HTTP/1.1 201", buf, MSG_LEN) == 0;
  }
  return ok;
}

void verify() {
  //if (!client.connected())
  //  client.stop();
  if (!nbAccess.isAccessAlive()) {
    nbAccess.shutdown();
    connect();
  }
}

/* ***************** General *********** */
float battery() {
  int p = analogRead(A1);
  return (float)p * 0.014956;  // R1 = 1.2M; R2 = 330k
}

void idle() {
  for(;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}

void schedule() {
  uint8_t m = (rtc.getMinutes() / MIVL + 1) * MIVL;
  if (m == 60)
    m = 0;
  rtc.setAlarmMinutes(m);
}

void setup() {
  //pinMode(FET, OUTPUT);
  //digitalWrite(FET, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  socket.begin();
  delay(3000);

  scan();
  if (sid[0] == '\0')
    idle();

  Wire.begin();
  SHTC3.begin();

  connect();
  update();

  rtc.begin();
  rtc.setEpoch(nbAccess.getTime());

  rtc.setAlarmSeconds(0);
  rtc.enableAlarm(rtc.MATCH_MMSS);
  schedule();
  rtc.standbyMode();
}

void loop() {
  String s = String(rtc.getYear() + 2000);
  s += "-";
  s += PRNT2(rtc.getMonth());
  s += "-";
  s += PRNT2(rtc.getDay());
  s += "T";
  s += PRNT2(rtc.getHours());
  s += ":";
  s += PRNT2(rtc.getMinutes());
  s += LF;

  float bat0 = battery();
  s += '$' + SIGN(bat0) + LF;

  SHTC3.readSample();
  float st = SHTC3.getTemperature();
  float rh = SHTC3.getHumidity();
  s += '%' + SIGN(st) + SIGN(rh) + LF;

  for (char *p = sid; *p; p++) {
    s += measure(*p);
  }

  verify();
  bool ok = post(s);
  delay(1000);
  s = String(ok);
  post(s);
  schedule();
  rtc.standbyMode();
}
