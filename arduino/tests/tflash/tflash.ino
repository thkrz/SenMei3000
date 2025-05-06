#include <SPI.h>
#include <SPIMemory.h>

#define CAP 1024

#define CS 7

#define BSZ 4  // (sizeof(uint32_t))
#define LF "\r\n"
#define LEN (addr[0])

static void dir();
static void discard();
static bool dump(String &);
static void erase();
static void insert(int n = 3);
static bool load(String &);
static void reprint();
static void sync();

SPIFlash flash(CS);
uint32_t addr[CAP];
String str;
uint32_t len;
int num;


void dir()
{
  Serial.println("DIR");
  for (int i = 0; i < CAP; i++)
    addr[i] = flash.readULong(i * BSZ);
}

void discard()
{
  Serial.println("DISCARD");
  if (LEN > 0)
    LEN--;
}

bool dump(String &s)
{
  Serial.print("DUMP: ");
  Serial.println(s);
  if (LEN < CAP - 1) {
    uint32_t a = flash.getAddress(flash.sizeofStr(s));
    if (a == 0)
      return false;
    addr[LEN + 1] = a;
    if (flash.writeStr(a, s)) {
      LEN++;
      sync();
      return true;
    }
  }
  return false;
}

void erase()
{
  Serial.println("ERASE");
  flash.eraseChip();
  for (int i = 0; i < CAP; i++) {
    flash.writeULong(i * BSZ, 0);
    addr[i] = 0;
  }
}

void insert(int n)
{
  static int i = 0;

  for (int j = 0; j < n; j++) {
    String s = "Teststring";
    s += i++;
    dump(s);
  }
}

bool load(String &s)
{
  Serial.println("LOAD");
  if (LEN < 1)
    return false;
  return flash.readStr(addr[LEN], s);
}

void reprint()
{
  String s;
  while (load(s)) {
    Serial.println(s);
    discard();
    sync();
  }
}

void sync()
{
  Serial.println("SYNC");
  while (!flash.eraseSector(0))
    ;
  for (int i = 0; i < CAP; i++)
    flash.writeULong(i * BSZ, addr[i]);
}

void setup()
{
  Serial.begin(19200);
  while (!Serial)
    ;
  delay(3000);

  flash.begin();
  //flash.powerDown();
}

void loop()
{
  if (Serial.available()) {
    Serial.println();
    char c = Serial.read();
    switch (c) {
    case 'd':
      len = LEN;
      while (load(str)) {
        Serial.println(str);
        discard();
      }
      LEN = len;
      Serial.println(F("#DUMP"));
      break;
    case 'w':
      flash.powerUp();
      Serial.println(F("#POWER UP"));
      break;
    case 'a':
      flash.powerDown();
      Serial.println(F("#POWER DOWN"));
      break;
    case 'f':
      erase();
      Serial.println(F("#CHIP ERASE"));
      break;
    case 'k':
      insert();
      Serial.println(F("#INSERT"));
      break;
    case 'l':
      dir();
      Serial.println(F("#DIR"));
      break;
    case 'r':
      reprint();
      Serial.println(F("#REPRINT"));
      break;
    case 'e':
      sync();
      Serial.println(F("#SYNC"));
      break;
    case 'i':
      Serial.print(F("M IN CACHE: "));
      Serial.println(LEN);
      num = 0;
      for (int i = 1; i < CAP; i++)
        if (addr[i] > 0)
          num++;
        else
          break;
      Serial.print(F("M ON FLASH: "));
      Serial.println(num);
      Serial.println(F("#INFO"));
      break;
    case '?':
      Serial.println(F("d: dump"));
      Serial.println(F("i: info"));
      Serial.println(F("f: chip erase"));
      Serial.println(F("e: sync"));
      Serial.println(F("w: power up"));
      Serial.println(F("a: power down"));
      Serial.println(F("l: dir"));
      Serial.println(F("k: insert"));
      Serial.println(F("r: reprint"));
      Serial.println(F("?: help"));
      break;
    }
  }
  delay(10);
}
