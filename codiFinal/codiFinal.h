
#include <Arduino.h>
//Include WiFi library
#include <WiFi.h>

//Include Firebase library (this library)
#include <Firebase_ESP_Client.h>

//llibreria per a la balança
#include "HX711.h"

#include <Ticker.h>

//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>

#define DATABASE_URL "https://smart-hob-a3dfc-default-rtdb.europe-west1.firebasedatabase.app/"

#define API_KEY "AIzaSyCYmv1rylWQUXyskwY6UBdKR4L8Qc_ZmtM"
#define USER_EMAIL "anna.dot@hotmail.es"
#define USER_PASSWORD "admin1234"

//#define WIFI_SSID "Xiaomi_B031"
//#define WIFI_PASSWORD "Casamarinera2"

#define WIFI_SSID "MOVISTAR_CD68"
#define WIFI_PASSWORD "VDot.1975_AnnaBannana"


//sensors capacitius
#define PIN_ON T4 //T0 24 //4
#define PIN_ADD T6 //T7 27
#define PIN_MINUS T5 //T3 15
#define PIN_P T3 //T4 13
#define PIN_M T7 //T5 12
#define PIN_B T0 //T6 14

//pins per a controlar els reles
#define RELE_ON 26
#define RELE_ADD 25
#define RELE_MINUS 33
#define RELE_P 32
#define RELE_M 16
#define RELE_B 17

//pin per controlar el buzzer
#define PIN_ALARM 5

//pins per a la balança
#define LOADCELL_DOUT_PIN 19
#define LOADCELL_SCK_PIN 18
