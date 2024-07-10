#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>

// WiFi configuration
#define WIFI_SSID "MEGACABLE-47A2"
#define WIFI_PASSWORD "8TYyKbJc"

// Firebase configuration
#define API_KEY "AIzaSyAf-6zUtowDnPI3FZbFEoT5HVJIzduKC00"
#define USER_EMAIL "binivazmor@gmail.com"
#define USER_PASSWORD "123456"
#define DATABASE_URL "https://morfo-esp32-test-f581b-default-rtdb.firebaseio.com/"

void authHandler();
void printError(int code, const String &msg);
void printResult(AsyncResult &aResult);

// Network and Firebase objects
DefaultNetwork network;
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(network));
RealtimeDatabase Database;
AsyncResult aResult_no_callback;

void setup()
{
    Serial.begin(115200);
    delay(2000); // Give time for Serial Monitor to open

    Serial.println("Starting setup...");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
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

    ssl_client.setInsecure();
    initializeApp(aClient, app, getAuth(user_auth), aResult_no_callback);
    authHandler();

    Serial.println("Authenticating user... ");
}

void loop()
{
    authHandler();
    Database.loop();
}

void authHandler()
{
    unsigned long ms = millis();
    while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000)
    {
        JWT.loop(app.getAuth());
        printResult(aResult_no_callback);
    }
    if (app.ready())
    {
        Serial.println("User authenticated successfully.");
        Serial.print("User UID: ");
        Serial.print(app.getUid());
    }
    else
    {
        Serial.println("User authentication failed.");
    }
}

void printResult(AsyncResult &aResult)
{
    if (aResult.isEvent())
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
        Serial.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    }
    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
        Serial.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }
    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
        Serial.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }
}

void printError(int code, const String &msg)
{
    Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
    Serial.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}
