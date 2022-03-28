class AC {

  volatile target::tc::Peripheral *tc;
  int step;
  bool polarity;
  PWM *pwm;
  int level;

  unsigned char acSine[generated::AC_TIMER_STEPS];

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

    setLevel(256);
  }

  void setLevel(int level) {
      for (int i = 0; i < generated::AC_TIMER_STEPS; i++) {
          acSine[i] = level * generated::AC_SINE[i] >> 24;
      }
  }

  void interruptHandlerTC() {
    if (tc->COUNT16.INTFLAG.getOVF()) {
      pwm->set(polarity, acSine[step]);
      if (step == generated::AC_TIMER_STEPS - 1) {
        step = 0;
        polarity = !polarity;
      } else {
        step++;
      }
      tc->COUNT16.INTFLAG.setOVF(true);
    }
  }
};