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
FirebaseData fb_write;
FirebaseData stream;

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
bool notifyFirebaseFocsAdd=false;
bool notifyFirebaseFocsMinus=false;
bool writeFirebase=true;
bool timer_0_state=false;
bool timer_1_state=false;
bool timer_2_state=false;


unsigned long sendDataPrevMillis = 0;
unsigned long vitroOnOff = 0;
unsigned long selFoc = 0;

class foc {
  private:
    String tamany;
    int8_t valor;
    unsigned long timer_f;
    unsigned long timer_i;
    
  public:
    //definicio del foc
    foc(String tamany) {
      this->tamany = tamany;
      this->valor = -1;
      this->timer_f=0;
      this->timer_i=0;
    }
    int8_t getValue() {
      return valor;
    }
    void updateValue(int8_t valor) {
      this->valor = valor;
    }
    void setTimer(unsigned long value_i, unsigned long value_f){
      this->timer_i=value_i;
      this->timer_f=value_f;
    }
    unsigned long getTimerF(){
      return this->timer_f;
    }
    unsigned long getTimerI(){
      return this->timer_i;
    }
};

class focs {
  private:
    foc array_focs[3] = {foc(ID_B), foc(ID_M),foc(ID_P)};
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
        array_focs[i].setTimer(0, 0);
      }
    }
    void setTimer(int8_t ID, unsigned long value_i, unsigned long value_f){
      array_focs[ID].setTimer(value_i, value_f);
    }
    unsigned long getValueTimerI (int8_t ID){
      return array_focs[ID].getTimerI();
    }
    unsigned long getValueTimerF (int8_t ID){
      return array_focs[ID].getTimerF();
    }
};

//creacio dels focs
focs Focs = focs();

//assignacio corresponent del nivell del foc
void ctr_numCtr(int8_t val, int8_t valFire, String ID){
   if(val!=-1){
     if(valFire>val){
      notifyFirebaseFocsAdd=true;
      notifyFirebaseFoc=ID;
      writeFirebase=false;
     }
     else if(val>valFire && (val!=9 || valFire!=0)){
      Serial.println(val);
      Serial.println(valFire);
      notifyFirebaseFocsMinus=true;
      notifyFirebaseFoc=ID;
      writeFirebase=false;
     }
   }
   else if(val==-1){
    if(valFire==5){
      notifyFirebaseFocsAdd=true;
      notifyFirebaseFoc=ID;
      writeFirebase=false;
    }
    else if (valFire==9){
      Serial.println(valFire);
      notifyFirebaseFocsMinus=true;
      notifyFirebaseFoc=ID;
      writeFirebase=false;
    }
 }
}

void streamCallback(FirebaseStream data)
{
  FirebaseJson *json = data.to<FirebaseJson *>();
  //Print all object data
  json->toString(Serial, true);
 
  FirebaseJsonData resultOn;
  json->get(resultOn, "/on");
  
  FirebaseJsonData resultNumCtr_0;
  json->get(resultNumCtr_0, "/0_state");

  FirebaseJsonData resultNumCtr_1;
  json->get(resultNumCtr_1, "/1_state");

  FirebaseJsonData resultNumCtr_2;
  json->get(resultNumCtr_2, "/2_state");

  FirebaseJsonData valueTimer;
  json->get(valueTimer, "/value");



  //si canvia algun on
  if (resultOn.success)
  {
    //Print its content e.g.string, int, double, bool whereas object, array and null also can access as string
    
    String dataPath=data.dataPath().c_str();
    //si canvia l'on general de la vitro
    if(dataPath=="/" && vitro_ON!=resultOn.to<int>()){
      notifyFirebaseOn=true;
    }
  }
  //si canvia algun 0_state
  if(resultNumCtr_0.success){ //mirar datapath data.dataPath().c_str()  
     Serial.println("SOC JOOOOOO");
     
     int8_t val=Focs.getValue(0);
     int8_t valFire=resultNumCtr_0.to<int>();
     String ID=ID_B;
     ctr_numCtr(val, valFire, ID);


  }
  if(resultNumCtr_1.success){
    int8_t val=Focs.getValue(1);
    int8_t valFire=resultNumCtr_1.to<int>();
    String ID=ID_M;
    ctr_numCtr(val, valFire, ID);
  }
  if(resultNumCtr_2.success){
    int8_t val=Focs.getValue(2);
    int8_t valFire=resultNumCtr_2.to<int>();
    String ID=ID_P;
    ctr_numCtr(val,valFire, ID);
  }

  if(valueTimer.success){ //timer
    String dataPath=data.dataPath().c_str();
    int8_t id_tmp;
    if(dataPath=="/timer/0_state"){ 
      id_tmp=0;
      //Serial.println("TIMEEER");
      //Serial.println();
      timer_0_state=true;
    }
    else if(dataPath=="/timer/1_state"){
      id_tmp=1;
      timer_1_state=true;
    }
    else{
      id_tmp=2;
      timer_2_state=true;
    }
    int8_t valueTimerInt= (valueTimer.to<String>()).toInt();
    unsigned long value_f = valueTimerInt * 60000; // min --> millis
    Focs.setTimer(id_tmp, millis(), value_f );  
  }
  
  json->clear();
  
}

void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

//per escriure de forma asíncrona a la firebase
void func_writeFirebase(String ruta, int8_t value){
  Firebase.RTDB.setIntAsync(&fb_write, ruta, value);
}

//per comprovar si ha passat el temps corresponent
void ctrTimers(int8_t ID, unsigned long vF, unsigned long vI){
  if(millis()-vI>=vF){ //si ha passat el temps, resetejem
    Serial.println("holiiii");
    if(ID==0){
      timer_0_state=false;
    }
    else if(ID==1){
      timer_1_state=false;
    }
    else{
      timer_2_state=false;
    }
    
    String ruta_write="/user1/vitro2545/timer/";
    ruta_write.concat(ID);
    ruta_write.concat("_state/value");
    func_writeFirebase(ruta_write, 0);
    
    ruta_write="/user1/vitro2545/timer/";
    ruta_write.concat(ID);
    ruta_write.concat("_state/on");
    func_writeFirebase(ruta_write, 0);
  }
  else{ 
    long vTempsPassat=vF-(millis()-vI);
    int valorFirebase=(vTempsPassat/60000)+1;
    String ruta_write="/user1/vitro2545/timer/";
    ruta_write.concat(ID);
    ruta_write.concat("_state/value");
    func_writeFirebase(ruta_write, valorFirebase);
  }
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
  
  //lectura asíncrona de l'estat de la vitro
  if (!Firebase.RTDB.beginStream(&stream, "/user1/vitro2545")){
    Serial.printf("sream begin error, %s\n\n", stream.errorReason().c_str());
  }
  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);

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

      func_writeFirebase("/user1/vitro2545/on", 1);
      
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
      func_writeFirebase("/user1/vitro2545/on", 0);
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
      if (cont_P==10 || notifyFirebaseFoc==ID_P) {
        ctr_foc = 2;

        notifyFirebaseFoc="";
        
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
      else if (cont_M==10 || notifyFirebaseFoc==ID_M) {
        ctr_foc = 1;

        notifyFirebaseFoc="";
        
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
        ctr_foc = 0;

        cont_B=0;
        cont_ON=0;
        cont_P=0;
        cont_M=0;
        cont_ADD=0;
        cont_MINUS=0;

        notifyFirebaseFoc="";
        
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
      else if (cont_ADD==10 || notifyFirebaseFocsAdd) {
        change=true;
        cont_B=0;
        cont_ON=0;
        cont_P=0;
        cont_M=0;
        cont_ADD=0;
        cont_MINUS=0;
        notifyFirebaseFocsAdd=false;
        Serial.println("ADD!!!!");
        if (ctr_foc != -1) {
          int8_t tmp_val = Focs.getValue(ctr_foc);
          if (tmp_val == -1) {
            Focs.setActive(ctr_foc, 5);
            Serial.println("5");
            if(writeFirebase){
              String ruta_write="/user1/vitro2545/num_control/";
              ruta_write.concat(ctr_foc);
              ruta_write.concat("_state");
              Serial.println(ruta_write);
              func_writeFirebase(ruta_write, 5);
            }
            else{
              writeFirebase=true;
            }
          }
          else if (tmp_val != 9) {
            Focs.setActive(ctr_foc, tmp_val + 1);
            Serial.println(tmp_val + 1);
            if(writeFirebase){
              String ruta_write="/user1/vitro2545/num_control/";
              ruta_write.concat(ctr_foc);
              ruta_write.concat("_state");
              Serial.println(ruta_write);
              func_writeFirebase(ruta_write, tmp_val+1);
            }
            else{
              writeFirebase=true;
            }
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
      else if (cont_MINUS==10 || notifyFirebaseFocsMinus) {
        cont_B=0;
        cont_ON=0;
        cont_P=0;
        cont_M=0;
        cont_ADD=0;
        cont_MINUS=0;

        notifyFirebaseFocsMinus=false;
        
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

            if(writeFirebase){
              String ruta_write="/user1/vitro2545/num_control/";
              ruta_write.concat(ctr_foc);
              ruta_write.concat("_state");
              func_writeFirebase(ruta_write, 9);
            }
            else{
              writeFirebase=true;
            }
          }
          else {
            Serial.println(tmp_val - 1);
            Focs.setActive(ctr_foc, tmp_val - 1);
            if(writeFirebase){
              String ruta_write="/user1/vitro2545/num_control/";
              ruta_write.concat(ctr_foc);
              ruta_write.concat("_state");
              func_writeFirebase(ruta_write, tmp_val-1);
            }
            else{
              writeFirebase=true;
            }
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

      //mirar els 3 timers
      if(timer_0_state){ //si el foc gran te timer
        unsigned long vI=Focs.getValueTimerI(0);
        unsigned long vF=Focs.getValueTimerF(0);
        ctrTimers(0,vF,vI);
      }
      if(timer_1_state){ //si el foc mitja te timer
        unsigned long vI=Focs.getValueTimerI(1);
        unsigned long vF=Focs.getValueTimerF(1);
        ctrTimers(1,vF,vI);
      }
      if(timer_2_state){ //si el foc petit te timer
        unsigned long vI=Focs.getValueTimerI(2);
        unsigned long vF=Focs.getValueTimerF(2);
        ctrTimers(2,vF,vI);
      }
    }
    //per apagar la vitro
    else if (!vitro_ON) {
      //reset de variables ctr
      change=false;
      vitroOnOff=0;
      selFoc=0;
      ctr_on = false;
      ctr_foc = -1;
      Focs.setAllInactive();
      notifyFirebaseFoc="";
      notifyFirebaseFocsAdd=false;
      notifyFirebaseFocsMinus=false;
      writeFirebase=true;

    }

  }
}
