/**
 * SYNTAX:
 *
 * T RealtimeDatabase::get<T>(<AsyncClient>, <path>);
 * T RealtimeDatabase::get<T>(<AsyncClient>, <path>, <DatabaseOptions>);
 *
 * T - The type of value to return.
 * <AsyncClient> - The async client.
 * <path> - The node path to get the value.
 * <DatabaseOptions> - The database options (DatabaseOptions).
 *
 * In case of data filtering and additional request options are required,
 * the DatabaseOptions can be assigned to the get function.
 *
 * Please see examples/Database/extras/FilteringData/FilteringData.ino for how to filter the data.
 *
 * The value of type T returns from sync Get functions is primitype type value or Arduino String.
 *
 * In case of error, the operation error information can be obtain from AsyncClient via aClient.lastError().message() and
 * aClient.lastError().code().
 *
 * Note that the objects used as placeholder (input) that introduced to use in Push, Update and Set i.e.
 * bool_t, number_t, string_t and object_t cannot be used to keep the output value.
 *
 * The complete usage guidelines, please visit https://github.com/mobizt/FirebaseClient
 */

#include <Arduino.h>
// #if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W) || defined(ARDUINO_GIGA)
#include <WiFi.h>
/*
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>) || defined(ARDUINO_NANO_RP2040_CONNECT)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>) || defined(ARDUINO_UNOWIFIR4)
#include <WiFiS3.h>
#elif __has_include(<WiFiC3.h>) || defined(ARDUINO_PORTENTA_C33)
#include <WiFiC3.h>
#elif __has_include(<WiFi.h>)
#include <WiFi.h>
#endif
*/
#include <FirebaseClient.h>

#define WIFI_SSID "MEGACABLE-47A2"
#define WIFI_PASSWORD "8TYyKbJc"

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "AIzaSyAf-6zUtowDnPI3FZbFEoT5HVJIzduKC00"

// User Email and password that already registerd or added in your project.
#define USER_EMAIL "binizavazquez@gmail.com"
#define USER_PASSWORD "Sagitario01@"
#define DATABASE_URL "https://morfo-esp32-test-f581b-default-rtdb.firebaseio.com/"

#define LED 2

void authHandler();

void printError(int code, const String &msg);

void printResult(AsyncResult &aResult);

DefaultNetwork network; // initilize with boolean parameter to enable/disable network reconnection

UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

FirebaseApp app;

// #if defined(ESP32) || defined(ESP8266) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;

using AsyncClient = AsyncClientClass;

AsyncClient aClient(ssl_client, getNetwork(network));

RealtimeDatabase Database;

AsyncResult aResult_no_callback;

void setup()
{

    pinMode(LED, OUTPUT);
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    unsigned long ms = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

    Serial.println("Initializing app...");

    // #if defined(ESP32) || defined(ESP8266) || defined(PICO_RP2040)
    ssl_client.setInsecure();
    /*



    /*
      Serial.print("Get string... ");
      String v3 = Database.get<String>(aClient, "/test/string");
      if (aClient.lastError().code() == 0)
          Serial.println(v3);
      else
          printError(aClient.lastError().code(), aClient.lastError().message());

      Serial.print("Get float... ");
      float v4 = Database.get<float>(aClient, "/test/float");
      if (aClient.lastError().code() == 0)
          Serial.println(v4);
      else
          printError(aClient.lastError().code(), aClient.lastError().message());

      Serial.print("Get double... ");
      double v5 = Database.get<double>(aClient, "/test/double");
      if (aClient.lastError().code() == 0)
          Serial.println(v5);
      else
          printError(aClient.lastError().code(), aClient.lastError().message());

          */
}

void loop()
{
    authHandler();

    Database.loop();
}

void authHandler()
{
    // Blocking authentication handler with timeout
    unsigned long ms = millis();
    while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000)
    {
        // This JWT token process required for ServiceAuth and CustomAuth authentications
        JWT.loop(app.getAuth());
        printResult(aResult_no_callback);
    }
}

void printResult(AsyncResult &aResult)
{
    if (aResult.isEvent())
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    }

    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }
}

void printError(int code, const String &msg)
{
    Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}