
#define PIN_ON T4 //T0 24 //4
#define PIN_ADD T6 //T2 2
#define PIN_MINUS T5 //T3 15
#define PIN_P T3 //T4 13
#define PIN_M T7 //T5 12 T7 27
#define PIN_B T0 //T6 14

const String ID_P = "state_0";
const String ID_M = "state_1";
const String ID_B = "state_2";

bool ctr_on = false;
//per saber l'ultim seleccionat
int8_t ctr_foc = -1;
bool vitro_ON=false;

int8_t cont_ON=0;
int8_t cont_P=0;
int8_t cont_M=0;
int8_t cont_B=0;
int8_t cont_ADD=0;
int8_t cont_MINUS=0;


unsigned long sendDataPrevMillis = 0;

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

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test");

}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() - sendDataPrevMillis > 50 || sendDataPrevMillis == 0) { //50
    //temporitzador
    sendDataPrevMillis = millis();

    //Serial.println("HI");

    int valueON = touchRead(PIN_ON);

    //si es pitja el boto ON/OFF
    ctr_on = ((valueON <= 20) && (valueON > 0) && !ctr_on);
    if(ctr_on){
      cont_ON++;

      cont_P=0;
      cont_M=0;
      cont_B=0;
      cont_ADD=0;
      cont_MINUS=0;
    }
    if(ctr_on && !vitro_ON && cont_ON==10){
      vitro_ON=true;
      cont_ON=0;
    }
    else if(ctr_on && vitro_ON && cont_ON==10){
      cont_ON=0;
      vitro_ON=false;
    }
    //Serial.println(valueON);
    //Serial.println(ctr_on);
    
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
      }
      else if((valueM <= 20) && (valueM > 0) ){
        cont_M++;
      }

      //si es selecciona el foc gran
      else if (cont_B==10 ) {
        ctr_foc = 2;
        
        cont_B=0;
        cont_ON=0;
        cont_P=0;
        cont_M=0;
        cont_ADD=0;
        cont_MINUS=0;
        
        Serial.println("B!!!!");
      }
      else if((valueB <= 20) && (valueB > 0)){
        cont_B++;
      }
      //si es pitja el +
      else if (cont_ADD==10) {
        cont_B=0;
        cont_ON=0;
        cont_P=0;
        cont_M=0;
        cont_ADD=0;
        cont_MINUS=0;
        
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
          int8_t tmp_val = Focs.getValue(ctr_foc);
          if (tmp_val == -1 || tmp_val == 0 ) {
            Serial.println("9");
            Focs.setActive(ctr_foc, 9);
          }
          else {
            Serial.println(tmp_val - 1);
            Focs.setActive(ctr_foc, tmp_val - 1);
          }
        }

      }
      else if((valueMINUS <= 20) && (valueMINUS > 0)){
        cont_MINUS++;
      }

    }
    //per apagar la vitro
    else if (!vitro_ON) {
      //reset de variables ctr
      ctr_on = false;
      ctr_foc = -1;
      Focs.setAllInactive();
      Serial.println("on --> off");
    }

    /*Serial.println(" ");
    Serial.println("---------------");
    Serial.println(" ");*/

  }
}
