#define VOLT(x) ((x) * 0.0048876)

float
temp(int v)
{
  float u = VOLT(v);
  return (u - 1.26) * 100.0 / 3.15;
}

float
moist(int v)
{
  float u = VOLT(v);
  return u * 100.0 / 3.15;
}

void
setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  delay(1000);
  Serial.println("Start...");
}

void
loop()
{
  analogRead(A0);
  analogRead(A1);
  int a1 = analogRead(A0);
  int a2 = analogRead(A1);

  Serial.print(temp(a1));
  Serial.print(" ");
  Serial.println(moist(a2));

  delay(500);
}
