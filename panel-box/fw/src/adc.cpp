
class ADC : public applicationEvents::EventHandler {

  int eventId;

  static void setPerpheralMux(int pin, target::port::PMUX::PMUXE mux) {
    target::PORT.PINCFG[pin].setPMUXEN(true);
    if (pin & 1) {
      target::PORT.PMUX[pin >> 1].setPMUXO((target::port::PMUX::PMUXO)mux);
    } else {
      target::PORT.PMUX[pin >> 1].setPMUXE(mux);
    }
  }

public:
  class Callback {
  public:
    virtual void adcConversionFinished() = 0;
  };

  int voltageConv = 0;
  Callback *callback;

  void init(int pin, target::port::PMUX::PMUXE mux, int ain,
            Callback *callback) {

    this->callback = callback;

    eventId = applicationEvents::createEventId();
    handle(eventId);

    setPerpheralMux(pin, mux);

    target::PM.APBCMASK.setADC(true);

    target::GCLK.CLKCTRL = target::GCLK.CLKCTRL.bare()
                               .setID(target::gclk::CLKCTRL::ID::ADC)
                               .setGEN(target::gclk::CLKCTRL::GEN::GCLK0)
                               .setCLKEN(true);

    while (target::GCLK.STATUS.getSYNCBUSY())
      ;

    target::ADC.INPUTCTRL =
        target::ADC.INPUTCTRL.bare()
            .setMUXNEG(target::adc::INPUTCTRL::MUXNEG::GND)
            .setMUXPOS(target::adc::INPUTCTRL::MUXPOS::PIN1);
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

    target::ADC.SWTRIG = target::ADC.SWTRIG.bare().setSTART(true);
  }

  void interruptHandlerADC() {
    if (target::ADC.INTFLAG.getRESRDY()) {
      voltageConv = target::ADC.RESULT;
      applicationEvents::schedule(eventId);
      target::ADC.SWTRIG = target::ADC.SWTRIG.bare().setSTART(true);
      target::PORT.OUTTGL = 1 << 8;
    }
  }

  void onEvent() { callback->adcConversionFinished(); }
};