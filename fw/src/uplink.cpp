

class Uplink : public genericTimer::Timer {

  volatile target::tc::Peripheral *tc;
  int pinDATA;

  int bitClock;

public:
  struct __attribute__((packed)) {
    unsigned char address;
    unsigned char protocol = 1;
    unsigned char error = 0;
    unsigned short vCap_mV = 0;
    unsigned char actDuty = 0;
    unsigned char maxDuty = 0;
  } txData;

  void init(int address, volatile target::tc::Peripheral *tc,
            target::gclk::CLKCTRL::GEN clockGen, int pinDATA) {

    this->tc = tc;
    this->pinDATA = pinDATA;

    txData.address = address;

    int tcIndex = ((int)(void *)tc - (int)(void *)&target::TC1) /
                  ((int)(void *)&target::TC2 - (int)(void *)&target::TC1);

    target::PM.APBCMASK.setTC(tcIndex + 1, true);

    target::GCLK.CLKCTRL = target::GCLK.CLKCTRL.bare()
                               .setID(target::gclk::CLKCTRL::ID::TC1_TC2)
                               .setGEN(clockGen)
                               .setCLKEN(true);

    target::PORT.DIRSET = 1 << pinDATA;

    tc->COUNT8.CTRLA =
        tc->COUNT8.CTRLA.bare()
            .setMODE(target::tc::COUNT8::CTRLA::MODE::COUNT8)
            .setPRESCALER(target::tc::COUNT8::CTRLA::PRESCALER::DIV1024);

    // 1kBd data rate from 48MHz clock
    tc->COUNT8.PER = tc->COUNT8.PER.bare().setPER(48E6 / 1024 / 1000);

    tc->COUNT8.INTENSET.setOVF(true);

    scheduleTransmit();
  }

  void scheduleTransmit() { start(100); }

  void interruptHandlerTC() {
    target::tc::COUNT8::INTFLAG::Register flags = tc->COUNT8.INTFLAG.copy();

    if (flags.getOVF()) {
      // target::PORT.OUTTGL = 1 << pinDATA;

      int clockPhase = bitClock & 1;
      int bitIndex = (bitClock >> 1) & 7;
      int byteIndex = bitClock >> 4;

      int out = ((((char*)(void*)&txData)[byteIndex] >> bitIndex) & 1) ^ clockPhase;

      if (out) {
        target::PORT.OUTSET = 1 << pinDATA;
      } else {
        target::PORT.OUTCLR = 1 << pinDATA;
      }      

      bitClock++;

      if (bitClock >> 4 == sizeof(txData)) {
        tc->COUNT8.CTRLA.setENABLE(false);
        scheduleTransmit();
      }
    }

    tc->COUNT8.INTFLAG = flags;
  }

  virtual void onTimer() {
    bitClock = 0;
    tc->COUNT8.CTRLA.setENABLE(true);
  }

} slave;
