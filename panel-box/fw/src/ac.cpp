class AC {

  volatile target::tc::Peripheral *tc;

public:
  void init(volatile target::tc::Peripheral *tc,
            target::gclk::CLKCTRL::GEN clockGen) {
    this->tc = tc;

    target::PM.APBCMASK.setTC(1, true);

    target::GCLK.CLKCTRL = target::GCLK.CLKCTRL.bare()
                               .setID(target::gclk::CLKCTRL::ID::TC1_TC2)
                               .setGEN(clockGen)
                               .setCLKEN(true);

    while (target::GCLK.STATUS.getSYNCBUSY())
      ;

    tc->COUNT16.CC[0].setCC(generated::AC_TIMER_STEPS - 1);
    tc->COUNT16.INTENSET = tc->COUNT16.INTENSET.bare().setOVF(true);
    tc->COUNT16.CTRLA = tc->COUNT16.CTRLA.bare()
                            .setENABLE(true)
                            .setWAVEGEN(target::tc::COUNT16::CTRLA::WAVEGEN::MFRQ)
                            .setMODE(target::tc::COUNT16::CTRLA::MODE::COUNT16)
                            .setPRESCALER(generated::AC_TIMER_PRESCALER);
  }

  void interruptHandlerTC() {
    target::PORT.OUTSET = 1 << 8;
//    for (volatile int c = 0; c < 100; c++);
    target::PORT.OUTCLR = 1 << 8;
    if (tc->COUNT16.INTFLAG.getOVF()) {
      tc->COUNT16.INTFLAG.setOVF(true);
    }
  }
};