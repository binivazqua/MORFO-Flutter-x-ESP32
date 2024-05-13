#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <SPIFFS.h>
#include "at_client.h"

#include "constants.h"

#define LED 2

// put function declarations here:
int myFunction(int, int);

void setup()
{
  // put your setup code here, to run once:

  // put your setup code here, to run once:

  // change this to the atSign you own and have the keys to
  const auto *at_sign = new AtSign("@bumpybluealiens");

  // reads the keys on the ESP32
  const auto keys = keys_reader::read_keys(*at_sign);

  // creates the AtClient object (allows us to run operations)
  auto *at_client = new AtClient(*at_sign, keys);

  // pkam authenticate into our atServer
  at_client->pkam_authenticate(SSID, PASSWORD);
}

void loop()
{
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }
}

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}