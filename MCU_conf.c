//Include WiFi library
//#include <FirebaseESP8266.h>
#include <WiFi.h>

//Include Firebase library (this library)
#include <Firebase_ESP_Client.h>

#define WIFI_SSID "Xiaomi_B031"
#define WIFI_PASSWORD "Casamarinera2"


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

}

void loop() {
  // put your main code here, to run repeatedly:

}
