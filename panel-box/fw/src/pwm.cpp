
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
  void init(volatile target::tcc::Peripheral *tcc, int pinQ1, target::port::PMUX::PMUXE muxQ1, int pinQ2, target::port::PMUX::PMUXE muxQ2, int pinQ3, target::port::PMUX::PMUXE muxQ3) {
    this->tcc = tcc;

    setPerpheralMux(pinQ1, muxQ1);
    setPerpheralMux(pinQ2, muxQ2);
    setPerpheralMux(pinQ3, muxQ3);

    tcc->PER = tcc->PER.bare().setPER(199); // gives ~40kHz CC=100 for 50% duty
    tcc->WAVE = tcc->WAVE.bare().setWAVEGEN(target::tcc::WAVE::WAVEGEN::NPWM);

    tcc->CTRLA = tcc->CTRLA.bare()
                            .setPRESCALER(target::tcc::CTRLA::PRESCALER::DIV1)
                            .setENABLE(true);
   
    while (tcc->SYNCBUSY)
      ;
  }

  void set(unsigned int channel, unsigned int duty) {
    tcc->CC[channel].setCC(duty);
  }
};