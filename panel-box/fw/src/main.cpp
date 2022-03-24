const int PIN_LED = 8;
const int PIN_SAFE_BOOT = 9;

const int PIN_SLAVE_SDA = 14;
const int PIN_SLAVE_SCL = 15;

const int PIN_MASTER_SDA = 22;
const int PIN_MASTER_SCL = 23;

const int PIN_SYNC = 16;
const int EXT_IN_SYNC = 0;

const int PIN_SENSE_I = 2;
const int PIN_SENSE_U = 3;

const int PIN_PWM_Q1 = 4;
const int PIN_PWM_Q2 = 5;
const int PIN_PWM_Q3 = 6;

enum Command { NONE = 0, ENABLE = 1 };

class Device {
public:
  class : public atsamd::i2c::Slave {
  public:
    struct __attribute__((packed)) {
      unsigned char command = Command::NONE;
      union {
        struct __attribute__((packed)) {
          unsigned char enabled;
        } enable;
      };
    } rxBuffer;

    Device *that;

    void init(Device *that) {
      this->that = that;
      Slave::init(0x50, 0, atsamd::i2c::AddressMode::MASK, &target::SERCOM0,
                  target::gclk::CLKCTRL::GEN::GCLK0, PIN_SLAVE_SDA,
                  target::port::PMUX::PMUXE::C, PIN_SLAVE_SCL,
                  target::port::PMUX::PMUXE::C);
    }

    virtual int getTxByte(int index) {
      return index < sizeof(that->state)
                 ? ((unsigned char *)&that->state)[index]
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

  struct __attribute__((packed)) {
    unsigned char duty = 10;
    bool direction : 1;
    bool endStop1 : 1;
    bool endStop2 : 1;
    bool reserved : 5;
    int actSteps;
    short currentMA = 0xFFFF;
  } state;

  void init() {

    // // TC1 for motor PWM

    // target::PM.APBCMASK.setTC(1, true);

    // target::GCLK.CLKCTRL = target::GCLK.CLKCTRL.bare()
    //                            .setID(target::gclk::CLKCTRL::ID::TC1_TC2)
    //                            .setGEN(target::gclk::CLKCTRL::GEN::GCLK0)
    //                            .setCLKEN(true);

    // while (target::GCLK.STATUS.getSYNCBUSY())
    //   ;

    slave.init(this);
  }

  void checkState() {

    bool running = state.duty != 0;
    target::PORT.OUTCLR.setOUTCLR(!running << PIN_LED);
    target::PORT.OUTSET.setOUTSET(running << PIN_LED);
  }

  void setSpeed(unsigned int speed) {}
};

Device device;

void interruptHandlerSERCOM0() { device.slave.interruptHandlerSERCOM(); }
void interruptHandlerEIC() {}

void initApplication() {

  // enable safeboot
  atsamd::safeboot::init(PIN_SAFE_BOOT, false, PIN_LED);

  // MCU clocked at 8MHz
  target::SYSCTRL.OSC8M.setPRESC(target::sysctrl::OSC8M::PRESC::_1);
  genericTimer::clkHz = 8E6;

  device.init();

  // enable interrupts
  target::NVIC.IPR[target::interrupts::External::SERCOM0 >> 2].setPRI(
      target::interrupts::External::SERCOM0 & 0x03, 3);
  target::NVIC.ISER.setSETENA(1 << target::interrupts::External::SERCOM0);
  target::NVIC.ISER.setSETENA(1 << target::interrupts::External::EIC);
}
