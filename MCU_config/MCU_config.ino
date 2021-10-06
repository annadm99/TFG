//Include WiFi library
//#include <FirebaseESP8266.h>
#include <WiFi.h>

//Include Firebase library (this library)
#include <Firebase_ESP_Client.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>

#define DATABASE_URL "https://smart-hob-a3dfc-default-rtdb.europe-west1.firebasedatabase.app/"

#define API_KEY "AIzaSyCYmv1rylWQUXyskwY6UBdKR4L8Qc_ZmtM"
#define USER_EMAIL "anna.dot@hotmail.es"
#define USER_PASSWORD "admin1234"

#define WIFI_SSID "Xiaomi_B031"
#define WIFI_PASSWORD "Casamarinera2"



FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
      Serial.print(".");
      delay(500);
  }

  Serial.println("Connectat al WiFi!");

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;

  //per a reconnectar automaticament
  Firebase.reconnectWiFi(true);
  //size del buffer de la firebase a dwld
  fbdo.setResponseSize(4096);

  //per comprovar l'estat del tokken
  config.token_status_callback = tokenStatusCallback; 
  /** Assign the maximum retry of token generation */
  config.max_token_generation_retry = 5;
  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Firebase.ready()){
    Serial.println("hiii");
  }

}
