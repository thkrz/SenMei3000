#include <MKRNB.h>
#include <RTCZero.h>
#include <SDI12.h>
#include <SHTC3.h>
#include <SPI.h>
#include <SPIMemory.h>
#include <Wire.h>

#include "config.h"
#include "global.h"

#define CAP 1024
#define FET 0
#define MX  1
#define RX  4
#define TX  3
#define MOD 5
#define CS  7

#define BSZ (sizeof(uint32_t))
#define LF "\r\n"
#define LEN (addr[0])
#define SIGN(x) (((x)>=0?'+':'\0')+String(x))
#define PRNT2(x) (((x)<10?'0':'\0')+String(x))

GPRS gprs;
NBClient client;
NB nbAccess;
RTCZero rtc;
SDI12 socket(MX, RX, TX);
SPIFlash flash(CS);
uint32_t addr[CAP];
char sid[63];

float battery() {
  int p = analogRead(A1);
  return (float)p * 0.014956;  // R1 = 1.2M; R2 = 330k
}

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

void dir() {
  for (int i = 0; i < CAP; i++)
    addr[i] = flash.readULong(i*BSZ);
}

bool discard() {
  if (LEN < 0)
    return false;
  LEN--;
  return true;
}

void disable() {
  socket.end();
  digitalWrite(FET, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

bool dump(String s) {
  if (LEN < CAP) {
    uint32_t a = flash.getAddress(flash.sizeofStr(s));
    if (a == 0)
      return false;
    addr[LEN+1] = a;
    if (flash.writeStr(a, s)) {
      LEN++;
      sync();
      return true;
    }
  }
  return false;
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(FET, HIGH);
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
  delay(30);
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

bool load(String &s) {
  if (LEN < 1)
    return false;
  uint32_t a = addr[LEN];
  return flash.readStr(a, s);
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

bool post(String s) {
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

    String r = "";
    while (!ok && client.available()) {
      r += (char)client.read();
      ok = r == "HTTP/1.1 201";
    }
  }
  return ok;
}

void pullup() {
  static int8_t pin[11] = {
    A0, A2, A3, A4, A5, A6, 5
  };

  for (int i = 0; i < 11; i++)
    pinMode(pin[i], INPUT_PULLUP);
}

void scan() {
  int n = 0;
  for (char c = '0'; c <= '9'; c++) {
    if (handshake(c))
      sid[n++] = c;
    delay(30);
  }
  for (char c = 'A'; c <= 'Z'; c++) {
    if (handshake(c))
      sid[n++] = c;
    delay(30);
  }
  for (char c = 'a'; c <= 'z'; c++) {
    if (handshake(c))
      sid[n++] = c;
    delay(30);
  }
  sid[n] = '\0';
}

void schedule() {
  uint8_t m = (rtc.getMinutes() / MIVL + 1) * MIVL;
  if (m == 60)
    m = 0;
  rtc.setAlarmMinutes(m);
}

void switchmode() {
  Serial.begin(9600);
  while (!Serial);

  for (;;) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '!') {
        String s;
        while(load(s)) {
          Serial.print(s);
          discard();
        }
        dir();
        Serial.print("#");
      }
    }
    delay(100);
  }
}

void sync() {
  while (!flash.eraseSector(0));
  for (int i = 0; i < CAP; i++)
    flash.writeULong(i*BSZ, addr[i]);
}

bool update() {
  String s = "UPDATE\r\n";
  enable();
  for (char *p = sid; *p; p++)
    s += ident(*p);
  disable();
  return post(s);
}

void verify() {
  //if (!client.connected())
  //  client.stop();
  if (!nbAccess.isAccessAlive()) {
    nbAccess.shutdown();
    connect();
  }
}

void setup() {
  pinMode(FET, OUTPUT);
  digitalWrite(FET, LOW);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pullup();

  if (digitalRead(MOD) == LOW)
    switchmode();
    /* not reached */

  delay(10000);

  enable();
  scan();
  disable();

  if (sid[0] == '\0')
    idle();

  flash.begin();
  dir();
  flash.powerDown();

  Wire.begin();
  SHTC3.begin();

  connect();
  update();

  rtc.begin();
  rtc.setEpoch(nbAccess.getTime());

  rtc.setAlarmSeconds(0);
  schedule();
  rtc.enableAlarm(rtc.MATCH_MMSS);
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
  s += "_" + SIGN(bat0) + LF;

  bool pm = bat0 < BAT_LOW;

  SHTC3.readSample(true, pm);
  float st = SHTC3.getTemperature();
  float rh = SHTC3.getHumidity();
  s += "." + SIGN(st) + SIGN(rh) + LF;

  enable();
  for (char *p = sid; *p; p++)
    s += measure(*p);
  disable();

  flash.powerUp();
  if (pm) {
    nbAccess.shutdown();
    dump(s);
  } else {
    verify();
    String p;
    bool sent = false;
    while(load(p)) {
      if(!post(p))
        break;
      sent = discard();
    }
    if (sent)
      sync();
    if (!post(s))
      dump(s);
  }

  if (!pm)
    schedule();
  flash.powerDown();
  rtc.standbyMode();
}
