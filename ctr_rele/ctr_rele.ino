#define RELE_ON 26
#define RELE_ADD 25
#define RELE_MINUS 33
#define RELE_P 32
#define RELE_M 35
#define RELE_B 34




void setup() {
  // put your setup code here, to run once:
  pinMode(RELE_ON, OUTPUT);
  pinMode(RELE_ADD, OUTPUT);
  pinMode(RELE_MINUS, OUTPUT);
  pinMode(RELE_P, OUTPUT);
  pinMode(RELE_M, OUTPUT);
  pinMode(RELE_B, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(RELE_ON,HIGH);
  delay(500);
  digitalWrite(RELE_ON,LOW);
  delay(500);
}
