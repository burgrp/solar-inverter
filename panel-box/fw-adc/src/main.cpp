const int PIN_LED = 8;
const int PIN_SAFE_BOOT = 9;

const int PIN_SLAVE_SDA = 14;
const int PIN_SLAVE_SCL = 15;
const target::port::PMUX::PMUXE MUX_SLAVE_SDA = target::port::PMUX::PMUXE::C;
const target::port::PMUX::PMUXE MUX_SLAVE_SCL = target::port::PMUX::PMUXE::C;

// const int PIN_MASTER_SDA = 22;
// const int PIN_MASTER_SCL = 23;
// const target::port::PMUX::PMUXE MUX_MASTER_SDA =
// target::port::PMUX::PMUXE::C; const target::port::PMUX::PMUXE MUX_MASTER_SCL
// = target::port::PMUX::PMUXE::C;

// const int PIN_SYNC = 16;
// const int EXT_IN_SYNC = 0;

const int PIN_SENSE_U_IN = 2;
const int PIN_SENSE_U_OUT = 3;
const int PIN_SENSE_I = 4;

const int AIN_SENSE_U_IN = 0;
const int AIN_SENSE_U_OUT = 1;
const int AIN_SENSE_I = 2;

const int PIN_CHARGE = 7;

// const int PIN_PWM_Q1 = 4;
// const int PIN_PWM_Q2 = 5;
// const int PIN_PWM_Q3 = 6;
// const target::port::PMUX::PMUXE MUX_PWM_Q1 = target::port::PMUX::PMUXE::F;
// const target::port::PMUX::PMUXE MUX_PWM_Q2 = target::port::PMUX::PMUXE::F;
// const target::port::PMUX::PMUXE MUX_PWM_Q3 = target::port::PMUX::PMUXE::F;

class Device : public ADC::Callback {
public:
  Uplink uplink;
  ADC adc;
  Reg reg;

  void init() {

    // enable safeboot

    atsamd::safeboot::init(PIN_SAFE_BOOT, false, PIN_LED);

    // MCU clocked at 8MHz

    target::SYSCTRL.OSC8M.setPRESC(target::sysctrl::OSC8M::PRESC::_1);
    genericTimer::clkHz = 8E6;

    // TCC0 clocked at 8MHz

    // target::PM.APBCMASK.setTCC0(true);

    // target::GCLK.CLKCTRL = target::GCLK.CLKCTRL.bare()
    //                            .setID(target::gclk::CLKCTRL::ID::TCC0)
    //                            .setGEN(target::gclk::CLKCTRL::GEN::GCLK0)
    //                            .setCLKEN(true);

    // while (target::GCLK.STATUS.getSYNCBUSY())
    //   ;

    // initialize subsystems

    uplink.init(8, &target::SERCOM0, target::gclk::CLKCTRL::GEN::GCLK0,
                PIN_SLAVE_SDA, MUX_SLAVE_SDA, PIN_SLAVE_SCL, MUX_SLAVE_SCL);

    adc.init(AIN_SENSE_U_IN, AIN_SENSE_I, this);

    reg.init(PIN_CHARGE);

    // enable interrupts

    target::NVIC.IPR[target::interrupts::External::SERCOM0 >> 2].setPRI(
        target::interrupts::External::SERCOM0 & 0x03, 3);
    target::NVIC.ISER.setSETENA(1 << target::interrupts::External::SERCOM0);
    target::NVIC.ISER.setSETENA(1 << target::interrupts::External::ADC);
  }

  void adcRead(int ain, int value) {
    if (ain == AIN_SENSE_U_IN) {
      uplink.state.inputVoltage = value;
    } else if (ain == AIN_SENSE_U_OUT) {
      uplink.state.outputVoltage = value;
      uplink.state.version++;
      reg.checkOutput(value);
    } else if (ain == AIN_SENSE_I) {
      uplink.state.outputCurrent = value;
    }
  }
};

Device device;

void interruptHandlerSERCOM0() { device.uplink.interruptHandlerSERCOM(); }
void interruptHandlerADC() { device.adc.interruptHandlerADC(); }

void initApplication() { device.init(); }
