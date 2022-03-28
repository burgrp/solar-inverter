
class ADC {

  static void setPerpheralMux(int pin, target::port::PMUX::PMUXE mux) {
    target::PORT.PINCFG[pin].setPMUXEN(true);
    if (pin & 1) {
      target::PORT.PMUX[pin >> 1].setPMUXO((target::port::PMUX::PMUXO)mux);
    } else {
      target::PORT.PMUX[pin >> 1].setPMUXE(mux);
    }
  }

public:
  static const int MAX_INPUTS = 10;
  const int AIN_TO_PIN[MAX_INPUTS] = {2, 3, 4, 5, 6, 7, 14, 15, 10, 11};

  int firstAin;
  int lastAin;

  class Callback {
  public:
    virtual void adcRead(int ain, int value) = 0;
  };

  int input = 0;

  Callback *callback;

  void init(int firstAin, int lastAin, Callback *callback) {

    this->firstAin = firstAin;
    this->lastAin = lastAin;
    this->callback = callback;

    for (int ain = firstAin; ain <= lastAin; ain++) {
      setPerpheralMux(AIN_TO_PIN[ain], target::port::PMUX::PMUXE::B);
    }

    target::PM.APBCMASK.setADC(true);

    target::GCLK.CLKCTRL = target::GCLK.CLKCTRL.bare()
                               .setID(target::gclk::CLKCTRL::ID::ADC)
                               .setGEN(target::gclk::CLKCTRL::GEN::GCLK0)
                               .setCLKEN(true);

    while (target::GCLK.STATUS.getSYNCBUSY())
      ;

    target::ADC.INPUTCTRL = target::ADC.INPUTCTRL.bare().setMUXNEG(
        target::adc::INPUTCTRL::MUXNEG::GND);
    target::PORT.PINCFG[3].setINEN(true);
    target::ADC.INTENSET = target::ADC.INTENSET.bare().setRESRDY(true);

    target::ADC.CALIB =
        target::ADC.CALIB.bare()
            .setBIAS_CAL(target::NVMCALIB.SOFT1.getADC_BIASCAL())
            .setLINEARITY_CAL(
                target::NVMCALIB.SOFT0.getADC_LINEARITY_LSB() ||
                (target::NVMCALIB.SOFT1.getADC_LINEARITY_MSB() >> 5));
    target::ADC.CTRLB =
        target::ADC.CTRLB.bare().setRESSEL(target::adc::CTRLB::RESSEL::_8BIT);
    target::ADC.AVGCTRL =
        target::ADC.AVGCTRL.bare()
            .setSAMPLENUM(target::adc::AVGCTRL::SAMPLENUM::_64_SAMPLES)
            .setADJRES(4);
    target::ADC.CTRLA = target::ADC.CTRLA.bare().setENABLE(true);

    target::ADC.INPUTCTRL.setMUXPOS((target::adc::INPUTCTRL::MUXPOS)firstAin);
    target::ADC.SWTRIG = target::ADC.SWTRIG.bare().setSTART(true);
  }

  void interruptHandlerADC() {
    if (target::ADC.INTFLAG.getRESRDY()) {
      int ain = (int)target::ADC.INPUTCTRL.getMUXPOS();
      int value = target::ADC.RESULT;
      callback->adcRead(ain, value);
      ain++;
      if (ain > lastAin) {
        ain = firstAin;
      }
      target::ADC.INPUTCTRL.setMUXPOS((target::adc::INPUTCTRL::MUXPOS)ain);
      target::ADC.SWTRIG = target::ADC.SWTRIG.bare().setSTART(true);
      target::PORT.OUTTGL = 1 << 8;
    }
  }

};