class AC {

  volatile target::tc::Peripheral *tc;
  int step;
  PWM *pwm;

public:
  void init(volatile target::tc::Peripheral *tc,
            target::gclk::CLKCTRL::GEN clockGen, PWM *pwm) {
    this->tc = tc;
    this->pwm = pwm;

    target::PM.APBCMASK.setTC(1, true);

    target::GCLK.CLKCTRL = target::GCLK.CLKCTRL.bare()
                               .setID(target::gclk::CLKCTRL::ID::TC1_TC2)
                               .setGEN(clockGen)
                               .setCLKEN(true);

    while (target::GCLK.STATUS.getSYNCBUSY())
      ;

    tc->COUNT16.CC[0].setCC(0);
    tc->COUNT16.INTENSET = tc->COUNT16.INTENSET.bare().setOVF(true);
    tc->COUNT16.CTRLA =
        tc->COUNT16.CTRLA.bare()
            .setENABLE(true)
            .setWAVEGEN(target::tc::COUNT16::CTRLA::WAVEGEN::MFRQ)
            .setMODE(target::tc::COUNT16::CTRLA::MODE::COUNT16)
            .setPRESCALER(generated::AC_TIMER_PRESCALER);

    start();
  }

  void start() { step = 0; }

  void interruptHandlerTC() {
    if (tc->COUNT16.INTFLAG.getOVF()) {
      pwm->set(0, 256 * generated::QUARTER_AC_CC[step >> 2] >> 24);
      if (step == generated::AC_TIMER_STEPS - 1) {
        step = 0;
        target::PORT.OUTSET = 1 << 8;
        target::PORT.OUTCLR = 1 << 8;

      } else {
        step++;
      }
      tc->COUNT16.INTFLAG.setOVF(true);
    }
  }
};