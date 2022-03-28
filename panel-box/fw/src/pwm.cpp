
/**
 * @brief PWM for power MOSFET switching
 * Assumes TCC is clocked on 8MHz
 */
class PWM {
  volatile target::tcc::Peripheral *tcc;

  static void setPerpheralMux(int pin, target::port::PMUX::PMUXE mux) {
    target::PORT.PINCFG[pin].setPMUXEN(true);
    if (pin & 1) {
      target::PORT.PMUX[pin >> 1].setPMUXO((target::port::PMUX::PMUXO)mux);
    } else {
      target::PORT.PMUX[pin >> 1].setPMUXE(mux);
    }
  }

public:
  void init(volatile target::tcc::Peripheral *tcc, int pin,
            target::port::PMUX::PMUXE mux, int count, int frequency) {
    this->tcc = tcc;

    for (int c = 0; c < count; c++) {
      setPerpheralMux(pin + c, mux);
    }

    tcc->PER = tcc->PER.bare().setPER(generated::PWM_TIMER_STEPS - 1);
    tcc->WAVE = tcc->WAVE.bare().setWAVEGEN(target::tcc::WAVE::WAVEGEN::NPWM);

    tcc->CTRLBSET = tcc->CTRLBSET.bare().setLUPD(true);

    tcc->CTRLA = tcc->CTRLA.bare()
                     .setPRESCALER(generated::PWM_TIMER_DIVIDER)
                     .setENABLE(true);

    while (tcc->SYNCBUSY)
      ;
  }

  void set(unsigned int polarity, unsigned int duty) {
    // tcc->CC[0].setCC(0);
    // tcc->CC[1].setCC(0);
    tcc->CC[polarity].setCC(duty);
  }
};