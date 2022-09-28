#include <cstddef>

class Uplink : public genericTimer::Timer {

  volatile target::tc::Peripheral *tc;
  int pinDATA;

  int bitClock;

public:
  typedef struct __attribute__((packed)) {
    unsigned char error = 0;
    unsigned short vCap_mV = 0;
    unsigned char actDuty = 0;
    unsigned char maxDuty = 0;
  } State;

  State state;

  struct __attribute__((packed)) {
    unsigned char preamble[2] = {0x55, 0x55};
    unsigned char address;
    unsigned char protocol = 1;
    State state;
    unsigned short crc;
  } buffer;

  void init(int address, volatile target::tc::Peripheral *tc,
            target::gclk::CLKCTRL::GEN clockGen, int pinDATA) {

    this->tc = tc;
    this->pinDATA = pinDATA;

    buffer.address = address;

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
    tc->COUNT8.PER = tc->COUNT8.PER.bare().setPER(48E6 / 1024 /* divider */
                                                  / 2         /* manchaster */
                                                  / 1000      /* baudrate */
    );

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

      int out =
          ((((char *)(void *)&buffer)[byteIndex] >> bitIndex) & 1) ^ clockPhase;

      if (out) {
        target::PORT.OUTSET = 1 << pinDATA;
      } else {
        target::PORT.OUTCLR = 1 << pinDATA;
      }

      bitClock++;

      if (bitClock >> 4 == sizeof(buffer)) {
        tc->COUNT8.CTRLA.setENABLE(false);
        scheduleTransmit();
      }
    }

    tc->COUNT8.INTFLAG = flags;
  }

  virtual void onTimer() {
    // trigger - REMOVE THIS
    target::PORT.DIRSET = 1 << 15;
    target::PORT.OUTSET = 1 << 15;
    target::PORT.OUTCLR = 1 << 15;

    buffer.state = state;
    unsigned short crc = 0x1235;
    for (int i = sizeof(buffer.preamble); i < (int)(void*)&buffer.crc - (int)(void*)&buffer; i++) {
      crc += ((char*)(void*)&buffer)[i];
    }
    buffer.crc = crc;

    bitClock = 0;
    tc->COUNT8.CTRLA.setENABLE(true);
  }

} slave;
