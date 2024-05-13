#include <Arduino.h>

#define LED 0

// put function declarations here:
int myFunction(int, int);

void setup()
{
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
}

void loop()
{
  // put your main code here, to run repeatedly:
  digitalWrite(LED, HIGH);
}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}