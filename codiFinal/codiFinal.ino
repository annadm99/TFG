#include "codiFinal.h"
#include "focs.h"

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

bool change_bF=true;
bool change_mF=true;
bool change_pF=true;


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

int valorFirebaseOld[3]={-1,-1,-1};

//per a saber si hi ha un timer setejat
bool timer_0_state=false;
bool timer_1_state=false;
bool timer_2_state=false;

//per saber si la balança esta oberta o no
bool valueBalanOn=false;

unsigned long sendDataPrevMillis = 0;
unsigned long vitroOnOff = 0;
unsigned long selFoc = 0;

unsigned long selFocB = 0;
unsigned long selFocM = 0;
unsigned long selFocP = 0;

TaskHandle_t TaskPesar_Handler;
unsigned long millisTaskPesar=0;
TaskHandle_t TaskPesAct_Handler;

//creacio dels focs
focs Focs = focs();

//creacio de la balança
HX711 balanca;


float balancaOldValue=-1;
float valueBalanca=0;
bool tarar_bal=false;

int temps_T_counter=0;

//assignacio corresponent del nivell del foc
void ctr_numCtr(int8_t val, int8_t valFire, String ID);

//per escriure de forma asíncrona a la firebase
void func_writeFirebase(String ruta, int value);

//gestio dels canvis en la firebase
void streamCallback(FirebaseStream data);
void streamTimeoutCallback(bool timeout); 

//funcio per a seleccionar el foc i activar el rele
void f_selecFoc(int8_t id, uint8_t pin);

//per comprovar si ha passat el temps corresponent dels timers
void ctrTimers(int8_t ID, unsigned long vF, unsigned long vI);

//per restar un nivell al foc
void f_minus();

void f_minus_t(int repeat);

Ticker ticker_temps;
Ticker ticker_rele;

//per afeguir un nivell al foc
void f_add();

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

  //configuracio del pin de l'alarma
  pinMode(PIN_ALARM, OUTPUT);
  
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

  //configuracio de la balança
  balanca.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  balanca.set_scale(402.17f); 
  balanca.tare();
  //balanca.power_down();

  /*xTaskCreatePinnedToCore(
    TaskPesar,
    "TaskPesar",
    8192,
    NULL,
    5,
    &TaskPesar_Handler,
    1
  );*/
  

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
      //delay(900);
      ticker_rele.once_ms(900, rele_H, RELE_ON);
      //digitalWrite(RELE_ON,HIGH);
      
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
      //delay(900);
      ticker_rele.once_ms(900, rele_H, RELE_ON);
      //digitalWrite(RELE_ON,HIGH);
    }
    //si s'ha rebut ordre d'encendre o apagar la vitro
    else if(notifyFirebaseOn){
      vitro_ON=!vitro_ON;
      digitalWrite(RELE_ON,LOW);
      //delay(900);
      ticker_rele.once_ms(900, rele_H, RELE_ON);
      //digitalWrite(RELE_ON,HIGH);
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
        f_selecFoc(2, RELE_P);
      }
      else if((valueP <= 20) && (valueP > 0)){
        cont_P++;
      }

      //si es selecciona el foc mitja
      else if (cont_M==10 || notifyFirebaseFoc==ID_M) {
        f_selecFoc(1, RELE_M);
      }
      else if((valueM <= 20) && (valueM > 0) ){
        cont_M++;
      }

      //si es selecciona el foc gran
      else if (cont_B==10 || notifyFirebaseFoc==ID_B) {
        f_selecFoc(0, RELE_B);
      }
      else if((valueB <= 20) && (valueB > 0)){
        cont_B++;
      }
      //si es pitja el +
      else if (cont_ADD==10 || notifyFirebaseFocsAdd) {
        f_add();
      }
      else if((valueADD <= 20) && (valueADD > 0)){
        cont_ADD++;
      }
      //si es pitja el -
      else if (cont_MINUS==10 || notifyFirebaseFocsMinus) {
        f_minus();

      }
      else if((valueMINUS <= 20) && (valueMINUS > 0)){
        cont_MINUS++;
      }

      //si no hi ha cap foc seleccionat es mira si han passat 9 seg
      if((ctr_foc==-1) && !change && ((millis()-vitroOnOff)>= 9000)){
        Serial.println("aix");
        vitro_ON=false;
        func_writeFirebase("/user1/vitro2545/on", 0);

      }
      
      //si hi ha un foc seleccionat i cap canvi, es mira si han passat 9 seg
      else if(ctr_foc!=-1 ){
        
        if(!change_bF && (millis()-selFocB>= 9000) ){
          Serial.println("he entrat B -------");
          change_bF=true;
          Focs.setInactive(0);
          String ruta_write="/user1/vitro2545/flag/flag";
          ruta_write.concat(0);
          func_writeFirebase(ruta_write, 1);
          func_writeFirebase(ruta_write, 0);
        }
        else if(!change_mF && (millis()-selFocM>= 9000)){
          Serial.println("he entrat M -------");
          change_mF=true;
          Focs.setInactive(1);
          String ruta_write="/user1/vitro2545/flag/flag";
          ruta_write.concat(1);
          func_writeFirebase(ruta_write, 1);
          func_writeFirebase(ruta_write, 0);
        }
        else if(!change_pF && (millis()-selFocP>= 9000)){
          Serial.println("he entrat P -------");
          change_pF=true;
          Focs.setInactive(2);
          String ruta_write="/user1/vitro2545/flag/flag";
          ruta_write.concat(2);
          func_writeFirebase(ruta_write, 1);
          func_writeFirebase(ruta_write, 0);
        }
  
        if(!change && (millis()-selFoc>= 9000)){
          Focs.setAllInactive();
          ctr_foc=-1;
          //func_writeFirebase(ruta_write, 0);
          vitroOnOff=millis();
        }
      }
      
      if(change && ctr_foc!=-1 && (millis()-selFoc>= 9000)){
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

      change_bF=true;
      change_mF=true;
      change_pF=true;
      
      vitroOnOff=0;
      selFoc=0;

      selFocB=0;
      selFocM=0;
      selFocP=0;
      
      ctr_on = false;
      ctr_foc = -1;
      Focs.setAllInactive();
      notifyFirebaseFoc="";
      notifyFirebaseFocsAdd=false;
      notifyFirebaseFocsMinus=false;
      writeFirebase=true;
      timer_0_state=false;
      timer_1_state=false;
      timer_2_state=false;

    }

    //si la balança esta oberta
    if(valueBalanOn){
      //realitzar les lectures
      //Serial.println("balançaaaa");
      //balanca.tare();
      Serial.println("reading-......");
      //realitzar 10 lectures i realitzar la mitja
      //float valueBalancaTemp=balanca.get_units(10);
      //Serial.println(valueBalancaTemp);

      //si el valor canvia en 3gr o mes , actualitzar el valor de la Firebase
      /*if(abs(valueBalancaTemp-balancaOldValue)>=2 || (round(valueBalancaTemp)==0 && balancaOldValue !=0) || abs(balancaOldValue)==1 ){
        Serial.println("ACTUALITZOOOOOOOOOOOOOOOOOOOOOOOOOOO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        balancaOldValue=round(valueBalancaTemp);
        func_writeFirebase("/user1/vitro2545/balan%C3%A7a/pes/", balancaOldValue);
      }
      func_writeFirebase("/user1/vitro2545/balan%C3%A7a/pes/", balancaOldValue);*/
      updateBalanca();
    }

  }
}


void updateBalanca(){
  if(abs(valueBalanca-balancaOldValue)>=2 || (round(valueBalanca)==0 && balancaOldValue !=0) || abs(balancaOldValue)==1 ){
    Serial.println("ACTUALITZOOOOOOOOOOOOOOOOOOOOOOOOOOO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    balancaOldValue=valueBalanca;
    func_writeFirebase("/user1/vitro2545/balan%C3%A7a/pes/", balancaOldValue);
  }

}
void TaskPesar(void *pvParameters) {
  //vTaskSuspend(NULL);
  balanca.tare();
  millisTaskPesar=millis();

  while(valueBalanOn) { // A Task shall never return or exit.
    if(tarar_bal){
      tarar_bal=false;
      balanca.tare();
    }
    else{
      valueBalanca=balanca.get_units(10);
    }
    Serial.println(valueBalanca);
    
    vTaskDelay(10/portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}


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

//gestio dels canvis en la firebase
void streamCallback(FirebaseStream data)
{
  
  FirebaseJson *json = data.to<FirebaseJson *>();
  //Print all object data
  json->toString(Serial, true);
  Serial.println(xPortGetCoreID());
  
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
  
  FirebaseJsonData valueBalanca;
  json->get(valueBalanca, "/pes");


  //si canvia algun on
  if (resultOn.success)
  {
    //Print its content e.g.string, int, double, bool whereas object, array and null also can access as string
    
    String dataPath=data.dataPath().c_str();
    //si canvia l'on general de la vitro
    if(dataPath=="/" && vitro_ON!=resultOn.to<int>()){
      notifyFirebaseOn=true;
    }
    //si canvia l'on de la balança
    else if(dataPath=="/balança"){
      valueBalanOn=resultOn.to<int>();
      Serial.println(valueBalanOn);
      Serial.println("ON BAL");

      if(!valueBalanOn){
        balancaOldValue=-1;
        //balanca.power_down();
        //vTaskSuspend(TaskPesar_Handler);
        //vTaskDelete(TaskPesar_Handler);
        Serial.println("Off BAL");
      }
      else{
        //balanca.power_up();
        //delay(1000);
        balanca.tare();
        //vTaskResume (TaskPesar_Handler);
        xTaskCreate(
          TaskPesar,
          "TaskPesar",
          8192,
          NULL,
          1,
          &TaskPesar_Handler
          );     
      }
      //notifyFirebaseOn=true;
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
    Serial.println("TIMEEER");
  }

  //per tarar la balança
  if(valueBalanca.success){
    Serial.println("TARAR");
    tarar_bal=true;
  }
  
  json->clear();
  
}

//per escriure de forma asíncrona a la firebase
void func_writeFirebase(String ruta, int value){
  Firebase.RTDB.setIntAsync(&fb_write, ruta, value);
}

void streamTimeoutCallback(bool timeout) 
{
  if (timeout)
    Serial.println("stream timed out, resuming...\n");

  if (!stream.httpConnected())
    Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
}

//funcio per a seleccionar el foc i activar el rele
void f_selecFoc(int8_t id, uint8_t pin){
  ctr_foc = id;

  notifyFirebaseFoc="";
  
  cont_M=0;
  cont_ON=0;
  cont_P=0;
  cont_B=0;
  cont_ADD=0;
  cont_MINUS=0;
  
  digitalWrite(pin,LOW);
  delay(600);
  //ticker_rele.once_ms(900,rele_H, pin);
  digitalWrite(pin,HIGH);
  /*if(id==0){
    selFocB=millis();
  }
  else if(id==1){
    selFocM=millis();
  }
  else{
    selFocP=millis();
  }*/
  selFoc=millis();
  Serial.println("sel!!!!!!!!!!!!!!!!!!");
  Serial.printf("PIN %i\n", pin);
}

void rele_H(int pin){
  digitalWrite(pin,HIGH);
}
//per restar un nivell al foc
void f_minus(){
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
    //delay(400);
    ticker_rele.once_ms(400, rele_H, RELE_MINUS);
    //digitalWrite(RELE_MINUS,HIGH);
    
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

      if((tmp_val-1)==0){
        if(ctr_foc==0){
          change_bF=false;
          selFocB=millis();

        }
        else if(ctr_foc==1){
          change_mF=false;
          selFocM=millis();
        }
        else{
          change_pF=false;
          selFocP=millis();
        }
      }
    }
    //mirar si els 3 focs estan a 0
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
    /*if(ctr_foc==0){
      selFocB=millis();
    }
    else if(ctr_foc==1){
      selFocM=millis();
    }
    else{
      selFocP=millis();
    }*/
    selFoc=millis();
  }
}
void f_minus_t(int repeat){
  if(temps_T_counter!=repeat){
    temps_T_counter++;
    f_minus();
  }
  else{
    ticker_temps.detach();
    temps_T_counter=0;
  }
}
//reset de les variables i afeguir un nivell al foc
void f_add(){
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
    //delay(400);
    ticker_rele.once_ms(400, rele_H, RELE_ADD);

    //digitalWrite(RELE_ADD,HIGH);
    /*if(ctr_foc==0){
      selFocB=millis();  
    }
    else if(ctr_foc==1){
      selFocM=millis();
    }
    else{
      selFocP=millis();
    }*/
    selFoc=millis();
  }
}

//per comprovar si ha passat el temps corresponent dels timers
void ctrTimers(int8_t ID, unsigned long vF, unsigned long vI){

  if(millis()-vI>=vF){ //si ha passat el temps, resetejem
    Serial.println("holiiii");
    uint8_t pin;
    if(ID==0){
      timer_0_state=false;
      pin=RELE_B;
    }
    else if(ID==1){
      timer_1_state=false;
      pin=RELE_M;
    }
    else{
      timer_2_state=false;
      pin=RELE_P;
    }
    valorFirebaseOld[ID]=-1;
    //actualitza la firebase
    String ruta_write="/user1/vitro2545/timer/";
    ruta_write.concat(ID);
    ruta_write.concat("_state/value");
    
    func_writeFirebase(ruta_write, 0);
    
    ruta_write="/user1/vitro2545/timer/";
    ruta_write.concat(ID);
    ruta_write.concat("_state/on");

    func_writeFirebase(ruta_write, 0);


    //digitalWrite(pin,LOW);
    //delay(900);
    //ticker_rele.once_ms(900,rele_H, pin);
    //digitalWrite(pin,HIGH);
    f_selecFoc(ID, pin);
    
    int temp_value= Focs.getValue(ID);

    ticker_temps.attach_ms(1200, f_minus_t, temp_value);
    /*for(int i=0; i<temp_value; i++){
      f_minus();
      delay(500);
    }*/
    //fer sonar el buzzer
    /*for(int i=0; i<3; i++){
      digitalWrite(PIN_ALARM, HIGH);    
      delay(1000);  
      digitalWrite(PIN_ALARM, LOW);
      delay(1000);
    }*/
  }
  else{ 
    long vTempsPassat=vF-(millis()-vI);
    int valorFirebase=(vTempsPassat/60000)+1;
    if(valorFirebase!=valorFirebaseOld[ID]){
      valorFirebaseOld[ID]=valorFirebase;
      String ruta_write="/user1/vitro2545/timer/";
      ruta_write.concat(ID);
      ruta_write.concat("_state/value");
      
      func_writeFirebase(ruta_write, valorFirebase);
    }
    
  }
}
