#include <MKRNB.h>
#include <RTCZero.h>
#include <SDI12.h>
#include <SHTC3.h>
#include <SPI.h>
#include <SPIMemory.h>
#include <Wire.h>

#include "config.h"
#include "global.h"

#define CS_PIN  7
#define DBG_PIN 5
#define FET_PIN 0
#define MUX_PIN 1
#define BUS_PIN 2

#define LP_MODE 11.6

GPRS gprs;
NBClient client;
NB nbAccess;
RTCZero rtc;
SDI12 socket(BUS_PIN);
SPIFlash flash;
char sid[63];
uint32_t addr = 0;
bool low_power = false;

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

void debug() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("DEBUG");
}

void disable() {
  socket.end();
  digitalWrite(FET_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);
}

void dump(String s) {
  flash.writeStr(addr, s);
  addr += s.length();
}

void enable() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(FET_PIN, HIGH);
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

void idle() {
  for(;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
}

String load() {
  static String s;

  if (flash.readStr(addr, s))
    addr = 0;
  return s;
}

String measure(char i) {
  static char st[4] = "aM!";
  static char rd[5] = "aD0!";
  static String s;

  socket.clearBuffer();
  delay(10);
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

bool post(String s) {
  bool ok = false;
  if (client.connect(HOST, PORT)) {
    client.println("POST "PATH"/"STAT_CTRL_ID" HTTP/1.1");
    client.println("Host: "HOST);
    client.println("Connection: close");
    client.println("Content-Type: text/plain");
    client.print("Content-Length: ");
    client.println(s.length());
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

void schedule() {
  uint8_t m = (rtc.getMinutes() / MIVL + 1) * MIVL;
  if (m == 60)
    m = 0;
  rtc.setAlarmMinutes(m);
}

String sprint02d(uint8_t d) {
  static String s;

  s = d < 10 ? "0" : "";
  s += String(d);
  return s;
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
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FET_PIN, OUTPUT);
  pinMode(MUX_PIN, OUTPUT);

  if (digitalRead(DBG_PIN) == HIGH)
    debug();

  enable();
  scan();
  disable();

  if (sid[0] == '\0')
    idle();

  flash.begin(CS_PIN);

  Wire.begin();
  SHTC3.begin();

  connect();

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
  s += sprint02d(rtc.getMonth());
  s += "-";
  s += sprint02d(rtc.getDay());
  s += "T";
  s += sprint02d(rtc.getHours());
  s += ":";
  s += sprint02d(rtc.getMinutes());
  s += "\r\n";

  float bat0 = battery();
  s += String(bat0) + "\r\n";

  low_power = bat0 < LP_MODE;

  SHTC3.readSample();
  s += String(SHTC3.getTemperature()) + "\r\n";
  s += String(SHTC3.getHumidity()) + "\r\n";

  enable();
  for (char *p = sid; *p; p++)
    s += measure(*p);
  disable();

  if (addr > 0)
    s += load();
  verify();
  if (!post(s))
    dump(s);

  schedule();
  rtc.standbyMode();
}
