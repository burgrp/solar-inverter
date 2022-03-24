

class Uplink: public atsamd::i2c::Slave {
  enum Command { NONE = 0, ENABLE = 1 };
  struct __attribute__((packed)) {
    unsigned char command = Command::NONE;
    union {
      struct __attribute__((packed)) {
        unsigned char enabled;
      } enable;
    };
  } rxBuffer;

public:
  struct __attribute__((packed)) {
    unsigned char error = 0xFF;
  } state;

  void init(int address, volatile target::sercom::Peripheral *sercom,
            target::gclk::CLKCTRL::GEN clockGen, int pinSDA,
            target::port::PMUX::PMUXE muxSDA, int pinSCL,
            target::port::PMUX::PMUXE muxSCL) {
    Slave::init(address, 0, atsamd::i2c::AddressMode::MASK, sercom, clockGen,
                pinSDA, muxSDA, pinSCL, muxSCL);
  }

  virtual int getTxByte(int index) {
    return index < sizeof(state) ? ((unsigned char *)&state)[index]
                                       : 0;
  }

  bool commandIs(Command command, int index, int value, int paramsSize) {
    return rxBuffer.command == command && index == paramsSize;
  }

  virtual bool setRxByte(int index, int value) {

    if (index < sizeof(rxBuffer)) {
      ((unsigned char *)&rxBuffer)[index] = value;

      if (commandIs(Command::ENABLE, index, value, sizeof(rxBuffer.enable))) {
      }

      return true;

    } else {
      return false;
    }
  }

} slave;
