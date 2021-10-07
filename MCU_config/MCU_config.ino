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



FirebaseData fb_readBalanca;
FirebaseData fb_write;
String parentPathBalanca = "/user1/vitro2545/balan%C3%A7a";
String childPathBalanca[2] = {"/on", "/pes"};

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

void streamCallbackBalanca(MultiPathStream stream)
{
  size_t numChild = sizeof(childPathBalanca) / sizeof(childPathBalanca[0]);
  

  for (size_t i = 0; i < numChild; i++)
  {
    Serial.println("SEPARACION");
    Serial.println(stream.get(childPathBalanca[i]));
    //comprovar si ha canviat el fill
    if (stream.get(childPathBalanca[i]))
    {
      Serial.printf("path: %s, event: %s, type: %s, value: %s%s", stream.dataPath.c_str(), stream.eventType.c_str(), stream.type.c_str(), stream.value.c_str(), i < numChild - 1 ? "\n" : "");
    }
  }

  Serial.println();

  //This is the size of stream payload received (current and max value)
  //Max payload size is the payload size under the stream path since the stream connected
  //and read once and will not update until stream reconnection takes place.
  //This max value will be zero as no payload received in case of ESP8266 which
  //BearSSL reserved Rx buffer size is less than the actual stream payload.
  Serial.printf("Received stream payload size: %d (Max. %d)\n\n", stream.payloadLength(), stream.maxPayloadLength());
}

void streamTimeoutCallbackBalanca(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!fb_readBalanca.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", fb_readBalanca.httpCode(), fb_readBalanca.errorReason().c_str());
}


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
  //fbdo.setResponseSize(4096);

  //per comprovar l'estat del tokken
  config.token_status_callback = tokenStatusCallback; 
  /** Assign the maximum retry of token generation */
  config.max_token_generation_retry = 5;
  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);
  
  //lectura asíncrona de la balança
  if (!Firebase.RTDB.beginMultiPathStream(&fb_readBalanca, parentPathBalanca)){
    Serial.printf("sream begin error, %s\n\n", fb_readBalanca.errorReason().c_str());
  }
  Firebase.RTDB.setMultiPathStreamCallback(&fb_readBalanca, streamCallbackBalanca, streamTimeoutCallbackBalanca);
  Serial.println("HOLAA");
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //cada quant s'executarà 
  if(Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0) ){
    //Serial.println("hiii");
    sendDataPrevMillis = millis();
    //escriptura
    Firebase.RTDB.setIntAsync(&fb_write, "/user1/vitro2545/on", 55);
  }

}
