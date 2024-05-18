#include <WiFi.h>
#include <Firebase_ESP_Client.h>

/* WIFI CREDENTIALS */
#define SSID "MEGACABLE-47A2"
#define PASSWORD "8TYyKbJc"

/* define API KEY */
#define API_KEY "AIzaSyAf-6zUtowDnPI3FZbFEoT5HVJIzduKC00"


/* define RIBDQ Key */
#define DATABASE_URL "https://morfo-esp32-test-f581b-default-rtdb.firebaseio.com/"

/* define user email and password */
#define USER_EMAIL "binizavazquez@gmail.com"
#define USER_PASSWORD "Sagitario01@"

/* define FIREBASE data object */
Firebase fbdo;

Firebase auth;
FirebaseConfig config;

const int LED = 2;
unsigned long sendDataPrevMillis = 0;



void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  Serial.begin(115200);

  /* INITIALIZE WIFI CREDENTIALS */
  WiFi.begin(SSID, PASSWORD);

  Serial.print("Connected to WiFi!");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.print(WiFi.localIP());
  Serial.println();

  /* Assign the api key as required */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* 
}

void loop() {
  // put your main code here, to run repeatedly:


}
