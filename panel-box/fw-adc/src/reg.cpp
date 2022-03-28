class Reg {

public:

    int pinCharge;

  void init(int pinCharge) {
      this->pinCharge = pinCharge;
      target::PORT.DIRSET = 1 << pinCharge;      
  }

  void checkOutput(int value) {
      if (value < 0x50) {
          target::PORT.OUTSET = 1 << pinCharge;
      } else if (value > 0x50) {
          target::PORT.OUTCLR = 1 << pinCharge;
      }
      
  }
};