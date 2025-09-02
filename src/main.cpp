#include <Arduino.h>

int myFunction(int, int);

void setup()
{
  Serial.println("Hello World!");
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(921600);
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("LED ON");
  delay(500);

  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("LED OFF");
  delay(500);
}
