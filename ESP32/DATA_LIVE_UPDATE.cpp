#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseClient.h>

#define WIFI_SSID "iPhone de Biniza Verónica"
#define WIFI_PASSWORD "cheesy00f"
#define API_KEY "AIzaSyAf-6zUtowDnPI3FZbFEoT5HVJIzduKC00"
#define USER_EMAIL "binizavazquez@gmail.com"
#define USER_PASSWORD "Sagitario01@"
#define DATABASE_URL "https://morfo-esp32-test-f581b-default-rtdb.firebaseio.com/"

#define LED 2

void authHandler();
void printError(int code, const String &msg);
void printResult(AsyncResult &aResult);

DefaultNetwork network;
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;
#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(network));
RealtimeDatabase Database;
AsyncResult aResult_no_callback;

// **************+ TIMERS ****************** //
unsigned long previous_millis = 0;
const long check_interval = 2000; // --> intervalo de 2 segundos.

void setup()
{
    pinMode(LED, OUTPUT);
    Serial.begin(115200);
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
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);

    Serial.println("Firebase && ESP32 setup COMPLETE.");
}

// ESPLICASION. 

/*
La combinación de la función loop() y su función interna de millis() 
junto con el authHandler() function make sure que la ESP32 está
authenticated con Firebase y pueda leer valores con certeza de forma
continua y regular.
 */

void loop()
{

    unsigned long current_milliseconds = millis();
    // checar si el último intervalo (2 segundos) ya pasaron desde el último check.
    if (current_milliseconds - previous_millis  >= check_interval) {
      
      // si ya pasó el intervalo, entonces previous_millis se acualiza.
      previous_millis = current_milliseconds;

      authHandler();
      /* HOW AND WHY? 
        1. Sets a period of 120 secs.
        2. Enters a loop until the Firebase App initializes or that period 
        has passed.
        3. Calls the JWT authentication process.
        4. prints the result of the auth process.
      */

      // Check if node exists:
      bool existed = Database.existed(aClient, "/LEDON");
      // Check the database for the LEDON value
      bool ledState = Database.get<bool>(aClient, "/LEDON");

        // IF !pedos con la sync a Firebase:
       if (aClient.lastError().code() == 0) {

          // check that the node exists:
          if (existed){
            // look for the value in the node:
             digitalWrite(LED, ledState ? HIGH : LOW);
             Serial.print("LEDON is ");
             Serial.println(ledState ? "true" : "false");

             
          } else {
            // no hay nodo compatible.
            Serial.println("WARNING! NO NODE MATCHES"); 
          }
          
       }

    } else {
      printError(aClient.lastError().code(), aClient.lastError().message());
    }   


    /* WHY?
    Database.loop();
    It is usually called within the main loop to process 
    any pending operations and handle background tasks related 
    to the Firebase connection, such as maintaining the connection 
    or processing incoming data.

    WHY NOT?
    because the simplified approach using periodic
     polling does not rely on asynchronous handling. 
     Instead, it synchronously checks the value of 
     LEDON from the database at regular intervals
    */


   
}

void authHandler()
{
    unsigned long ms = millis();
    while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000)
    {
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
    if (code != 0){
      Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
    }
    
}
