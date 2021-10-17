//Include WiFi library
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

const String ID_B = "state_0";
const String ID_M = "state_1";
const String ID_P = "state_2";

//firebase
FirebaseData fb_readOn;
FirebaseData fb_write;
String parentPathOn = "/user1/vitro2545/";
String childPathOn[2] = {"/on","/num_control/0_state"};


FirebaseAuth auth;
FirebaseConfig config;


//control de la vitro
bool ctr_on = false;
//per saber l'ultim seleccionat
int8_t ctr_foc = -1;
bool vitro_ON=false;
bool change=false;

int8_t cont_ON=0;
int8_t cont_P=0;
int8_t cont_M=0;
int8_t cont_B=0;
int8_t cont_ADD=0;
int8_t cont_MINUS=0;

bool notifyFirebaseOn=false;
String notifyFirebaseFoc="";
bool notifyFirebaseFocsAdd;


unsigned long sendDataPrevMillis = 0;
unsigned long vitroOnOff = 0;
unsigned long selFoc = 0;

class foc {
  private:
    String tamany;
    int8_t valor;
  public:
    //definicio del foc
    foc(String tamany) {
      this->tamany = tamany;
      this->valor = -1;
    }
    int8_t getValue() {
      return valor;
    }
    void updateValue(int8_t valor) {
      this->valor = valor;
    }
};

class focs {
  private:
    foc array_focs[3] = {foc(ID_P), foc(ID_M), foc(ID_B)};
  public:
    void setActive(int8_t id, int8_t value) {
      array_focs[id].updateValue(value);
    }
    int8_t getValue(int8_t id) {
      return array_focs[id].getValue();
    }
    void setInactive(int8_t id) {
      array_focs[id].updateValue(-1);
    }
    void setAllInactive() {
      for (int i = 0; i < 3; i++) {
        array_focs[i].updateValue(-1);
      }
    }
};

//creacio dels focs
focs Focs = focs();


void streamCallbackOn(MultiPathStream stream)
{
  size_t numChild = sizeof(childPathOn) / sizeof(childPathOn[0]);
  //Serial.println(numChild);

  for (size_t i = 0; i < numChild; i++)
  {
    Serial.println("SEPARACION");
    Serial.println(stream.get(childPathOn[i]));
    //comprovar si ha canviat el fill
    if (stream.get(childPathOn[i]))
    {
      //Serial.printf("path: %s, event: %s, type: %s, value: %s%s \n", stream.dataPath.c_str(), stream.eventType.c_str(), stream.type.c_str(), stream.value.c_str(), i < numChild - 1 ? "\n" : "");
      if(vitro_ON!=stream.value.toInt() && i==0){
        notifyFirebaseOn=true;
        /*Serial.println("aquiiii");
        cont_ON=10;*/
      }
      else{
        Serial.println("pffff");
        if(i==1){
          int8_t val=Focs.getValue(i-1);
          int8_t valFire=stream.value.toInt();
         Serial.println(valFire);
          if(valFire==5){
            notifyFirebaseFocsAdd=true;
            notifyFirebaseFoc=ID_B;
          }
          else if(valFire==9){
            notifyFirebaseFocsAdd=false;
            notifyFirebaseFoc=ID_B;
          }
          else if(val!=-1){
            notifyFirebaseFocsAdd=val>valFire?false:true;
            notifyFirebaseFoc=ID_B;
          }
          
        }

        
      }

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

void streamTimeoutCallbackOn(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!fb_readOn.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", fb_readOn.httpCode(), fb_readOn.errorReason().c_str());
}





void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test");


  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  
  //configuracio dels pins dels reles
  pinMode(RELE_ON, OUTPUT);
  pinMode(RELE_ADD, OUTPUT);
  pinMode(RELE_MINUS, OUTPUT);
  pinMode(RELE_P, OUTPUT);
  pinMode(RELE_M, OUTPUT);
  pinMode(RELE_B, OUTPUT);

  digitalWrite(RELE_ON,HIGH);
  digitalWrite(RELE_ADD,HIGH);
  digitalWrite(RELE_MINUS,HIGH);
  digitalWrite(RELE_P,HIGH);
  digitalWrite(RELE_M,HIGH);
  digitalWrite(RELE_B,HIGH);


  
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

  //per comprovar l'estat del tokken
  config.token_status_callback = tokenStatusCallback; 
  /** Assign the maximum retry of token generation */
  config.max_token_generation_retry = 5;
  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);
  
  //lectura asíncrona de la balança
  if (!Firebase.RTDB.beginMultiPathStream(&fb_readOn, parentPathOn)){
    Serial.printf("sream begin error, %s\n\n", fb_readOn.errorReason().c_str());
  }
  Firebase.RTDB.setMultiPathStreamCallback(&fb_readOn, streamCallbackOn, streamTimeoutCallbackOn);


  
  Serial.println("HOLAA");

}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() - sendDataPrevMillis > 50 || sendDataPrevMillis == 0) { //50
    //temporitzador
    sendDataPrevMillis = millis();

    int valueON = touchRead(PIN_ON);

    //si es pitja el boto ON/OFF
    ctr_on = ((valueON <= 20) && (valueON > 0));
    if(ctr_on){
      cont_ON++;

      cont_P=0;
      cont_M=0;
      cont_B=0;
      cont_ADD=0;
      cont_MINUS=0;
    }
    else{
      cont_ON=0;
    }
    if(ctr_on && !vitro_ON && cont_ON==10){
      vitro_ON=true;
      cont_ON=0;

      
      
      digitalWrite(RELE_ON,LOW);
      delay(900);
      digitalWrite(RELE_ON,HIGH);
      
      vitroOnOff=millis();

      //encendre la vitro amb el rele
      
      Serial.println("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOONNNNNNNNNNNNNNNNNNNNN");

    }
    else if(ctr_on && vitro_ON && cont_ON==10){
      cont_ON=0;
      vitro_ON=false;
      Serial.println("OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOFFFFFFFFFFF");
      digitalWrite(RELE_ON,LOW);
      delay(900);
      digitalWrite(RELE_ON,HIGH);
    }
    //si s'ha rebut ordre d'encendre o apagar la vitro
    else if(notifyFirebaseOn){
      vitro_ON=!vitro_ON;
      digitalWrite(RELE_ON,LOW);
      delay(900);
      digitalWrite(RELE_ON,HIGH);
      vitroOnOff=millis();
      notifyFirebaseOn=false;
    }
    //si esta oberta
    if (vitro_ON) {

      //Serial.println("ON");

      int valueADD = touchRead(PIN_ADD);
      int valueMINUS = touchRead(PIN_MINUS);
      int valueP = touchRead(PIN_P);
      int valueM = touchRead(PIN_M);
      int valueB = touchRead(PIN_B);

      //si es selecciona el foc petit
      if (cont_P==10) {
        ctr_foc = 0;

        cont_P=0;
        cont_ON=0;
        cont_M=0;
        cont_B=0;
        cont_ADD=0;
        cont_MINUS=0;

        Serial.println("P!!!!");

        digitalWrite(RELE_P,LOW);
        delay(900);
        digitalWrite(RELE_P,HIGH);

        selFoc=millis();
      }
      else if((valueP <= 20) && (valueP > 0)){
        cont_P++;
      }

      //si es selecciona el foc mitja
      else if (cont_M==10) {
        ctr_foc = 1;

        
        cont_M=0;
        cont_ON=0;
        cont_P=0;
        cont_B=0;
        cont_ADD=0;
        cont_MINUS=0;

        Serial.println("M!!!!");
        
        digitalWrite(RELE_M,LOW);
        delay(900);
        digitalWrite(RELE_M,HIGH);
        
        selFoc=millis();
      }
      else if((valueM <= 20) && (valueM > 0) ){
        cont_M++;
      }

      //si es selecciona el foc gran
      else if (cont_B==10 || notifyFirebaseFoc==ID_B) {
        ctr_foc = 2;

        cont_B=0;
        cont_ON=0;
        cont_P=0;
        cont_M=0;
        cont_ADD=0;
        cont_MINUS=0;

        notifyFirebaseFoc="-";
        
        Serial.println("B!!!!");

        digitalWrite(RELE_B,LOW);
        delay(900);
        digitalWrite(RELE_B,HIGH);
        
        selFoc=millis();
      }
      else if((valueB <= 20) && (valueB > 0)){
        cont_B++;
      }
      //si es pitja el +
      else if (cont_ADD==10 || (notifyFirebaseFoc!="" && notifyFirebaseFocsAdd)) {
        change=true;
        cont_B=0;
        cont_ON=0;
        cont_P=0;
        cont_M=0;
        cont_ADD=0;
        cont_MINUS=0;
        notifyFirebaseFoc="";
        Serial.println("ADD!!!!");
        if (ctr_foc != -1) {
          int8_t tmp_val = Focs.getValue(ctr_foc);
          if (tmp_val == -1) {
            Focs.setActive(ctr_foc, 5);
            Serial.println("5");
          }
          else if (tmp_val != 9) {
            Focs.setActive(ctr_foc, tmp_val + 1);
            Serial.println(tmp_val + 1);
          }

          digitalWrite(RELE_ADD,LOW);
          delay(400);
          digitalWrite(RELE_ADD,HIGH);

          selFoc=millis();
        }
        

      }
      else if((valueADD <= 20) && (valueADD > 0)){
        cont_ADD++;
      }
      else if (cont_MINUS==10) {
        cont_B=0;
        cont_ON=0;
        cont_P=0;
        cont_M=0;
        cont_ADD=0;
        cont_MINUS=0;


        Serial.println("MINUS!!!!");
        if (ctr_foc != -1) {
          
          digitalWrite(RELE_MINUS,LOW);
          delay(400);
          digitalWrite(RELE_MINUS,HIGH);
          
          int8_t tmp_val = Focs.getValue(ctr_foc);
          change=true;
          if (tmp_val == -1 || tmp_val == 0 ) {
            Serial.println("9");
            Focs.setActive(ctr_foc, 9);
          }
          else {
            Serial.println(tmp_val - 1);
            Focs.setActive(ctr_foc, tmp_val - 1);
          }
          int8_t counter_chng=0;
          for(int i=0; i<3; i++){
            int8_t tmp_val2 = Focs.getValue(i);
            if(tmp_val2 == -1 || tmp_val2 == 0){
              counter_chng++;
            }
          }
          if(counter_chng==3){
            change=false;
          }
          selFoc=millis();
        }

      }
      else if((valueMINUS <= 20) && (valueMINUS > 0)){
        cont_MINUS++;
      }

      //si no hi ha cap foc seleccionat es mira si han passat 9 seg
      if((ctr_foc==-1) && !change && ((millis()-vitroOnOff)>= 9000)){
        Serial.println("aix");
        vitro_ON=false;
        //Serial.println(vitroOnOff);
        //Serial.print(millis());



      }
      //si hi ha un foc seleccionat i cap canvi, es mira si han passat 9 seg
      else if(ctr_foc!=-1 && !change && (millis()-selFoc>= 9000)){
        ctr_foc=-1;
        vitroOnOff=millis();
      }
      else if(change && ctr_foc!=-1 && (millis()-selFoc>= 9000)){
        ctr_foc=-1;
      }
      else if(change){
        vitroOnOff=0;
      }

    }
    //per apagar la vitro
    else if (!vitro_ON) {
      //Serial.println("off");
      //reset de variables ctr
      change=false;
      vitroOnOff=0;
      selFoc=0;
      ctr_on = false;
      ctr_foc = -1;
      Focs.setAllInactive();
    }

  }
}
