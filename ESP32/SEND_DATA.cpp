#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseClient.h>

#define WIFI_SSID "MEGACABLE-47A2"
#define WIFI_PASSWORD "8TYyKbJc"
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

void setup() {
    pinMode(LED, OUTPUT);
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
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

    Serial.println("Synchronous Get... ");

    // recibir si está o no desde firebase:

    bool LEDStatus = Database.get<bool>(aClient, "/LEDOn"); // ASÍ INICIA, COMO LE PONGO EN FIREBASE.
    if (aClient.lastError().code() == 0) {
        Serial.print("LEDON: ");
        Serial.println(LEDStatus);
        digitalWrite(LED, LEDStatus ? HIGH : LOW); // BLINK ACCORDINGLY
    } else {
        printError(aClient.lastError().code(), aClient.lastError().message());
    }
}

void loop() {
    authHandler();
    Database.loop();

    /* CHAT EXPLICA QUE PEDO:

    La razón principal para declarar ledStatus como estática tanto en setup() como en loop() 
    es asegurar que tenga un valor inicial consistente al inicio del programa (setup()) 
    y que mantenga su valor a través de la ejecución continua del programa (loop()). 
    Esto proporciona coherencia en el manejo del estado del LED y evita problemas 
    relacionados con reinicializaciones no deseadas o cambios de estado imprevistos. */

    
    static bool LEDStatus = false; // chat addons -> variable OTRA VEZ ESTÁTICA que va acontrolar el LED dependiendo del estatus actualizado de FIREBASE.
    static unsigned long ultimo_blink = 0; // chat

    if (millis() - ultimo_blink >= 5000) { // que cambie cada 5 segundos dependiendo de el dato anterior que recibió.
        LEDStatus = !LEDStatus; // cambia.
        digitalWrite(LED, LEDStatus ? HIGH : LOW); 
        sendLED(LEDStatus); //enviar el estatus actualizado
        
        ultimo_blink = millis(); // jaló tons se queda
    }
}

void sendLED(bool status) {
    Serial.print("ESTATUS A FIREBASE: ");
    Serial.println(status);
    Database.set<bool>(aClient, "/LED_BLINK", status);
    if (aClient.lastError().code() != 0) {
        printError(aClient.lastError().code(), aClient.lastError().message());
    }
}

void authHandler() {
    unsigned long ms = millis();
    while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000) {
        JWT.loop(app.getAuth());
        printResult(aResult_no_callback);
    }
}

void printResult(AsyncResult &aResult) {
    if (aResult.isEvent()) {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    }
    if (aResult.isDebug()) {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }
    if (aResult.isError()) {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }
}

void printError(int code, const String &msg) {
    Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}

