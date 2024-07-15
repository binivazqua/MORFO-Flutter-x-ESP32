#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>

// WiFi configuration
#define WIFI_SSID "MEGACABLE-47A2"
#define WIFI_PASSWORD "8TYyKbJc"

// Firebase configuration
#define API_KEY "AIzaSyAf-6zUtowDnPI3FZbFEoT5HVJIzduKC00"
#define USER_EMAIL "binivazmor@gmail.com"
#define USER_PASSWORD "123456"
#define DATABASE_URL "https://morfo-esp32-test-f581b-default-rtdb.firebaseio.com/"

bool authHandler();
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
String userUID;

// NTP Client configuration
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -6 * 3600, 60000); // Update every 60 seconds

bool dataSent = false;
bool authPrinted = false;
bool taskComplete = false;

unsigned long previousMillis = 0;
const long interval = 3000; // Interval to simulate sensor readings (3 seconds)

void setup()
{
    Serial.begin(115200);
    delay(2000);

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

    timeClient.begin();

    Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
    Serial.println("Initializing Firebase app...");

    ssl_client.setInsecure();
    initializeApp(aClient, app, getAuth(user_auth), aResult_no_callback);
    app.getApp<RealtimeDatabase>(Database);

    Database.url(DATABASE_URL);
    authHandler();

    Serial.println("Authenticating user...");

    // Check authentication status
    if (app.ready())
    {
        if (!authPrinted)
        {
            Serial.println("User authenticated successfully.");
            userUID = app.getUid();
            Serial.print("User UID: ");
            Serial.println(userUID);
            authPrinted = true; // Ensure this message is printed only once
        }
    }
    else
    {
        Serial.println("User authentication failed.");
    }

    // Initialize NTP client and force an initial update
    timeClient.forceUpdate();

    Serial.println("Setup complete. Simulating egg sensor readings...");
}

void loop()
{
    unsigned long currentMillis = millis();

    Database.loop();
    app.loop();

    if (app.ready() && (currentMillis - previousMillis >= interval))
    {
        previousMillis = currentMillis;

        // Simulate egg sensor readings
        /*
        int eggValue1 = random(0, 1024); // Simulate a reading between 0 and 1023 for muscle group 1
        int eggValue2 = random(0, 1024); // Simulate a reading between 0 and 1023 for muscle group 2
        String timestamp = getCurrentTime();

        Serial.print("Simulated egg Value 1: ");
        Serial.println(eggValue1);
        Serial.print("Simulated egg Value 2: ");
        Serial.println(eggValue2);
        Serial.print("Timestamp: ");
        Serial.println(timestamp);

        // Create path for the data
        String path = "/sensorSim/" + userUID;

        Serial.print("Pushing data to path: ");
        Serial.println(path);
        */

        // Simulate EMG sensor reading
        int emgValue1 = random(0, 250); // Simulate a reading between 0 and 1023
        int emgValue2 = random(0, 250); // Simulate a reading between 0 and 1023

        Serial.print("Simulated EMG Value 1: ");
        Serial.println(emgValue1);

        Serial.print("Simulated EMG Value 2: ");
        Serial.println(emgValue2);

        Serial.print("Current date: ");
        Serial.println(getCurrentTime());

        // paths to be used:
        String sBasePath = "/sensorSim/";
        String UIDPath = sBasePath + "/" + app.getUid();
        String timePath = UIDPath + "/" + getCurrentTime();

        // create the user sensor path:
        Database.set<String>(aClient, timePath, "", aResult_no_callback);

        // upload sensor data:
        Database.push<int>(aClient, timePath + "/muscle 1", emgValue1, aResult_no_callback);
        Database.push<int>(aClient, timePath + "/muscle 2", emgValue2, aResult_no_callback);
        if (aClient.lastError().code() == 0)
        {
            Serial.println("Egg sensor values and timestamp uploaded successfully.");
        }
        else
        {
            Serial.print("Error uploading egg sensor values or timestamp. Code: ");
            Serial.println(aClient.lastError().code());
            Serial.print("Error message: ");
            Serial.println(aClient.lastError().message());
        }
    }

    printResult(aResult_no_callback);
}

String getCurrentTime()
{
    time_t rawTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime(&rawTime);
    char dateBuffer[20];
    sprintf(dateBuffer, "%04d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    String currentTime = String(dateBuffer);

    return currentTime;
}

bool authHandler()
{
    unsigned long ms = millis();
    while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000)
    {
        JWT.loop(app.getAuth());
        printResult(aResult_no_callback);
        delay(100); // Add a small delay to avoid flooding the Serial Monitor
    }

    if (app.ready())
    {
        Serial.println("User authenticated successfully.");
        Serial.print("User UID: ");
        Serial.println(app.getUid());
        return true;
    }
    else
    {
        Serial.println("User authentication failed.");
        return false;
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
