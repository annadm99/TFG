
#define PIN_ON T4 //T0 24 //4
#define PIN_ADD T6 //T2 2
#define PIN_MINUS T5 //T3 15
#define PIN_P T3 //T4 13
#define PIN_M T7 //T5 12 T7 27
#define PIN_B T0 //T6 14

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test");

}

void loop() {
  // put your main code here, to run repeatedly:

  int valueON=touchRead(PIN_ON);
  int valueADD=touchRead(PIN_ADD);
  int valueMINUS=touchRead(PIN_MINUS);
  int valueP=touchRead(PIN_P);
  int valueM= touchRead(PIN_M);
  int valueB=touchRead(PIN_B);
  
  if((valueON<=20) && (valueON>0)){
    Serial.println("ON");
  }
  else if((valueADD<=20) && (valueADD>0) ){
    Serial.println("ADD!!!!");
  }
  else if((valueMINUS<=20) && (valueMINUS>0)){
    Serial.println("MINUS!!!!");
  }
  else if((valueP<=20)&& (valueP>0)){
    Serial.println("P!!!!");
  }
  else if((valueM<=20)&&(valueM>0)){
    Serial.println("M!!!!");
  }
  else if((valueB<=20)&& (valueB>0)){
    Serial.println("B!!!!");
  }

  
}
