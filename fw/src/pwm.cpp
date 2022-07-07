class PWM {
  volatile target::tc::Peripheral *tc;

  static void setPerpheralMux(int pin, target::port::PMUX::PMUXE mux) {
    target::PORT.PINCFG[pin].setPMUXEN(true);
    if (pin & 1) {
      target::PORT.PMUX[pin >> 1].setPMUXO((target::port::PMUX::PMUXO)mux);
    } else {
      target::PORT.PMUX[pin >> 1].setPMUXE(mux);
    }
  }

public:

  int woIndex;
  int fullDuty;

  void init(volatile target::tc::Peripheral *tc,
            target::gclk::CLKCTRL::GEN clockGen, int pin,
            target::port::PMUX::PMUXE mux, int woIndex) {
    
    this->tc = tc;
    this->woIndex = woIndex;

    // 200kHz PWM out of 48MHz clock - make sure it fits in 8bits
    fullDuty = 48E6/200E3;

    int tcIndex = ((int)(void *)tc - (int)(void *)&target::TC1) /
                  ((int)(void *)&target::TC2 - (int)(void *)&target::TC1);
    target::PM.APBCMASK.setTC(tcIndex + 1, true);

    target::GCLK.CLKCTRL = target::GCLK.CLKCTRL.bare()
                               .setID(target::gclk::CLKCTRL::ID::TC1_TC2)
                               .setGEN(clockGen)
                               .setCLKEN(true);

    while (target::GCLK.STATUS.getSYNCBUSY())
      ;

    setPerpheralMux(pin, mux);

    tc->COUNT8.CTRLA =
        tc->COUNT8.CTRLA.bare()
            .setMODE(target::tc::COUNT8::CTRLA::MODE::COUNT8)
            .setWAVEGEN(target::tc::COUNT8::CTRLA::WAVEGEN::NPWM)
            .setPRESCALER(target::tc::COUNT8::CTRLA::PRESCALER::DIV1)
            .setENABLE(true);

    while (tc->COUNT8.STATUS.getSYNCBUSY())
      ;

    tc->COUNT8.PER =
        tc->COUNT8.PER.bare().setPER(fullDuty - 1);

    set(0);
  }

  void set(unsigned int duty) {
    tc->COUNT8.CC[woIndex] = duty;
  }
};