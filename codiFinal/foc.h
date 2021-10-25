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
