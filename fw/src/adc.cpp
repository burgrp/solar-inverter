
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
  class Input {
  public:
    int pin;
    int ain;
  };

  class Callback {
  public:
    virtual void adcRead(Input input, int value) = 0;
  };

  const Input *adcInputs;
  int adcInputsLen;

  int input = 0;

  Callback *callback;

  void init(const Input *adcInputs, int adcInputsLen, Callback *callback) {

    this->adcInputs = adcInputs;
    this->adcInputsLen = adcInputsLen;
    this->callback = callback;

    for (int i = 0; i < adcInputsLen; i++) {
      setPerpheralMux(adcInputs[i].pin, target::port::PMUX::PMUXE::B);
    }

    target::PM.APBCMASK.setADC(true);

    target::GCLK.CLKCTRL = target::GCLK.CLKCTRL.bare()
                               .setID(target::gclk::CLKCTRL::ID::ADC)
                               .setGEN(target::gclk::CLKCTRL::GEN::GCLK0)
                               .setCLKEN(true);

    while (target::GCLK.STATUS.getSYNCBUSY())
      ;

    target::ADC.INPUTCTRL = target::ADC.INPUTCTRL.bare()
                                .setMUXNEG(target::adc::INPUTCTRL::MUXNEG::IOGND)
                                .setGAIN(target::adc::INPUTCTRL::GAIN::DIV2);

    target::ADC.CALIB =
        target::ADC.CALIB.bare()
            .setBIAS_CAL(target::NVMCALIB.SOFT1.getADC_BIASCAL())
            .setLINEARITY_CAL(
                target::NVMCALIB.SOFT0.getADC_LINEARITY_LSB() |
                (target::NVMCALIB.SOFT1.getADC_LINEARITY_MSB() << 5));

    target::ADC.CTRLB = target::ADC.CTRLB.bare()
                            .setPRESCALER(target::adc::CTRLB::PRESCALER::DIV512)
                            .setRESSEL(target::adc::CTRLB::RESSEL::_8BIT);

    target::ADC.REFCTRL = target::ADC.REFCTRL.bare().setREFCOMP(true).setREFSEL(
        target::adc::REFCTRL::REFSEL::INT1V);

    target::ADC.SAMPCTRL = target::ADC.SAMPCTRL.bare().setSAMPLEN(0x3F);

    target::ADC.CTRLA = target::ADC.CTRLA.bare().setENABLE(true);
    target::ADC.INTENSET = target::ADC.INTENSET.bare().setRESRDY(true);

    startConversion();
  }

  void startConversion() {
    target::ADC.INPUTCTRL.setMUXPOS(
        (target::adc::INPUTCTRL::MUXPOS)adcInputs[input].ain);
    target::ADC.SWTRIG = target::ADC.SWTRIG.bare().setSTART(true);
  }

  void interruptHandlerADC() {
    if (target::ADC.INTFLAG.getRESRDY()) {
      int mV = (1000 * target::ADC.RESULT) >> 7;
      callback->adcRead(adcInputs[input], mV);
      input++;
      if (input == adcInputsLen) {
        input = 0;
      }
      startConversion();
    }
  }
};