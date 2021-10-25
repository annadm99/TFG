#include "foc.h"

class focs {
  private:
    foc array_focs[3] = {foc("state_0"), foc("state_1"),foc("state_2")};
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
