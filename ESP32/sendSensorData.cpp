#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>

// WiFi configuration
#define WIFI_SSID "iPhone de Biniza Verónica"
#define WIFI_PASSWORD "cheesy00f"

// #define WIFI_SSID "MEGACABLE-47A2"
// #define WIFI_PASSWORD "8TYyKbJc"

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
// set up USER'S UID.
String userUID;

// time configuration:
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -6 * 3600, 60000); // Update every 60 seconds

bool authPrinted = false;
bool taskComplete = false;

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
    Serial.println("Initializing app...");

    ssl_client.setInsecure();
    initializeApp(aClient, app, getAuth(user_auth), aResult_no_callback);
    app.getApp<RealtimeDatabase>(Database);

    Database.url(DATABASE_URL);
    authHandler();

    Serial.println("Authenticating user... ");

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
    timeClient.begin();
    timeClient.forceUpdate();
}

void loop()
{

    // prueba simple
    app.loop();
    Database.loop();

    String sensorDataPath = "/sensorData/";
    if (app.ready() && !taskComplete)
    {
        taskComplete = true;

        Serial.println("Asynchronous Set... ");

        // paths to be used:
        String sBasePath = "/sensorData/";
        String UIDPath = sBasePath + "/" + app.getUid();
        String timePath = UIDPath + "/" + getCurrentTime();
        String muscle1Path = UIDPath + "/" + getCurrentTime() + "/" + "muscle 1";
        String muscle2Path = UIDPath + "/" + getCurrentTime() + "/" + "muscle 2";

        Serial.print("Current date: ");
        Serial.println(getCurrentTime());

        // create the user sensor path:
        Database.set<String>(aClient, timePath, "", aResult_no_callback);

        // upload sensor data:
        Database.push<int>(aClient, muscle1Path, 3134, aResult_no_callback);
        Database.push<int>(aClient, muscle2Path, 3112, aResult_no_callback);

        if (aClient.lastError().code() == 0)
        {
            Serial.println("Values uploaded successfully.");
            taskComplete = true;
        }
        else
        {
            Serial.print("Error uploading EMG value. Code: ");
            Serial.println(aClient.lastError().code());
            Serial.print("Error message: ");
            Serial.println(aClient.lastError().message());
        }
    }

    printResult(aResult_no_callback);
}

void sendDatawithMonitor()
{
    // Only proceed if authenticated and data has not been sent yet
    if (app.ready() && !taskComplete)
    {
        // Update the time client to get the current time
        timeClient.update();

        // Check if data is available in the Serial Monitor
        if (Serial.available() > 0)
        {
            String input = Serial.readStringUntil('\n');
            int emgValue = input.toInt();
            Serial.print("EMG Value: ");
            Serial.println(emgValue);

            // Create path for sensor data
            String path = "/sensorData";

            // Debugging statement
            Serial.print("Setting data at path: ");
            Serial.println(path);
            Serial.print("With value: ");
            Serial.println(emgValue);

            // Upload EMG value to Firebase using set (overwriting previous value)
            Database.set(aClient, path, emgValue);
            if (aClient.lastError().code() == 0)
            {
                Serial.println("EMG value uploaded successfully.");
                taskComplete = true; // Set the flag to true to avoid sending data again
            }
            else
            {
                Serial.print("Error uploading EMG value. Code: ");
                Serial.println(aClient.lastError().code());
                Serial.print("Error message: ");
                Serial.println(aClient.lastError().message());
            }

            // Ensure we print only once
            while (true)
            {
                delay(1000);
            }
        }
    }
}

String getCurrentTime()
{
    // Get the current date

    time_t rawTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime(&rawTime);
    char dateBuffer[20];
    sprintf(dateBuffer, "%04d-%02d-%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
    String currentDate = String(dateBuffer);

    return currentDate;
}

bool authHandler()
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
