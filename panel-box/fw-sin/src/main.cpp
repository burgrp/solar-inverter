const int LED_PIN = 8;
const int SAFE_BOOT_PIN = 9;

const int SLAVE_SDA_PIN = 14;
const int SLAVE_SCL_PIN = 15;
const target::port::PMUX::PMUXE SLAVE_SDA_MUX = target::port::PMUX::PMUXE::C;
const target::port::PMUX::PMUXE SLAVE_SCL_MUX = target::port::PMUX::PMUXE::C;

const int SYNC_PIN = 16;
const int SYNC_EXTIN = 0;

const int SENSE_I_PIN = 2;
const int SENSE_U_PIN = 3;

const int PWM_PIN = 4;
const int PWM_COUNT = 2;
const target::port::PMUX::PMUXE PWM_MUX = target::port::PMUX::PMUXE::F;
const int PWM_FREQ = 120000;

class Device {
public:
  Uplink uplink;
  PWM pwm;
  AC ac;

  void init() {

    // enable safeboot

    atsamd::safeboot::init(SAFE_BOOT_PIN, false, LED_PIN);

    // MCU clocked at 8MHz

    genericTimer::clkHz = generated::CLK_FREQ_HZ;
    target::SYSCTRL.OSC8M.setPRESC(target::sysctrl::OSC8M::PRESC::_1);

    // initialize subsystems

    uplink.init(8, &target::SERCOM0, target::gclk::CLKCTRL::GEN::GCLK0,
                SLAVE_SDA_PIN, SLAVE_SDA_MUX, SLAVE_SCL_PIN, SLAVE_SCL_MUX);

    pwm.init(&target::TCC0, target::gclk::CLKCTRL::GEN::GCLK0, PWM_PIN, PWM_MUX, PWM_COUNT, PWM_FREQ);

    ac.init(&target::TC1, target::gclk::CLKCTRL::GEN::GCLK0, &pwm);

    // enable interrupts

    target::NVIC.IPR[target::interrupts::External::SERCOM0 >> 2].setPRI(
        target::interrupts::External::SERCOM0 & 0x03, 3);
    target::NVIC.ISER.setSETENA(1 << target::interrupts::External::SERCOM0);
    target::NVIC.ISER.setSETENA(1 << target::interrupts::External::EIC);
    target::NVIC.ISER.setSETENA(1 << target::interrupts::External::TC1);

  }
};

Device device;

void interruptHandlerSERCOM0() { device.uplink.interruptHandlerSERCOM(); }
void interruptHandlerTC1() {device.ac.interruptHandlerTC();}
void interruptHandlerEIC() {}

void initApplication() { device.init(); }
