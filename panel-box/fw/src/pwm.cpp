
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

    tcc->CTRLA = tcc->CTRLA.bare()
                            .setPRESCALER(target::tcc::CTRLA::PRESCALER::DIV1)
                            .setENABLE(true);

    tcc->PER.setPER(0xFFFFFF);

    while (tcc->SYNCBUSY)
      ;

    // needs to be 0xFE to achieve full-on on CC=0xFF
    // tc->COUNT8.PER.setPER(0xFE);
    // tc->COUNT8.CC[0].setCC(0);
  }

  void set(unsigned int dc) {
    // tc->COUNT8.CC[0].setCC(dc);
  }
};