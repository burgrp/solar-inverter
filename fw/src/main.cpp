const int LED_PIN = 8;
const int SAFE_BOOT_PIN = 9;

const int SLAVE_SDA_PIN = 14;
const int SLAVE_SCL_PIN = 15;
const target::port::PMUX::PMUXE SLAVE_SDA_MUX = target::port::PMUX::PMUXE::C;
const target::port::PMUX::PMUXE SLAVE_SCL_MUX = target::port::PMUX::PMUXE::C;

const int GATE_PIN = 5;
const target::port::PMUX::PMUXE GATE_MUX = target::port::PMUX::PMUXE::E;
const int GATE_WO_INDEX = 1;
const int GATE_FREQ = 120000;

const int VIN_R1 = 10000;
const int VIN_R2 = 270;
const int VIN_MULT = (VIN_R1 + VIN_R2) / VIN_R2;
const ADC::Input ADC_VIN = {pin : 2, ain : 0};

const int VOUT_R1 = 10000;
const int VOUT_R2 = 270;
const int VOUT_MULT = (VOUT_R1 + VOUT_R2) / VOUT_R2;
const ADC::Input ADC_VOUT = {pin : 4, ain : 2};

const ADC::Input ADC_INPUTS[2] = {ADC_VIN, ADC_VOUT};

class Device : public ADC::Callback {
public:
  Uplink uplink;
  PWM pwm;
  ADC adc;

  void init() {

    // enable safeboot

    atsamd::safeboot::init(SAFE_BOOT_PIN, false, LED_PIN);

    // DFLL48M -> GEN0 -> MAIN CLOCK

    target::NVMCTRL.CTRLB.setRWS(target::nvmctrl::CTRLB::RWS::HALF);

    target::SYSCTRL.DFLLCTRL.zero();
    target::SYSCTRL.DFLLVAL = target::SYSCTRL.DFLLVAL.bare().setCOARSE(
        target::NVMCALIB.SOFT1.getDFLL48M_COARSE_CAL());
    target::SYSCTRL.DFLLMUL = target::SYSCTRL.DFLLMUL.bare().setMUL(1);
    target::SYSCTRL.DFLLCTRL = target::SYSCTRL.DFLLCTRL.bare().setENABLE(true);
    while (!target::SYSCTRL.PCLKSR.getDFLLRDY())
      ;

    target::GCLK.GENCTRL = target::GCLK.GENCTRL.bare()
                               .setGENEN(true)
                               .setSRC(target::gclk::GENCTRL::SRC::DFLL48M)
                               .setIDC(true)
                               .setOE(true)
                               .setID(0);
    while (target::GCLK.STATUS.getSYNCBUSY())
      ;

    // MCU now clocked at 48MHz

    genericTimer::clkHz = 48E6;

    // debug - output generator 0 to PA24
    // target::PORT.PMUX[24 >> 1].setPMUXE(target::port::PMUX::PMUXE::H);
    // target::PORT.PINCFG[24].setPMUXEN(true).setDRVSTR(true);

    // initialize subsystems

    adc.init(ADC_INPUTS, sizeof(ADC_INPUTS) / sizeof(ADC::Input), this);

    uplink.init(8, &target::SERCOM0, target::gclk::CLKCTRL::GEN::GCLK0,
                SLAVE_SDA_PIN, SLAVE_SDA_MUX, SLAVE_SCL_PIN, SLAVE_SCL_MUX);

    pwm.init(&target::TC1, target::gclk::CLKCTRL::GEN::GCLK0, GATE_PIN,
             GATE_MUX, GATE_WO_INDEX, GATE_FREQ);

    // enable interrupts

    target::NVIC.IPR[target::interrupts::External::SERCOM0 >> 2].setPRI(
        target::interrupts::External::SERCOM0 & 0x03, 3);
    target::NVIC.ISER.setSETENA(1 << target::interrupts::External::SERCOM0);
    target::NVIC.ISER.setSETENA(1 << target::interrupts::External::ADC);
  }

  void adcRead(ADC::Input input, int scaled) {
    if (input.ain == ADC_VIN.ain) {
      int real = VIN_MULT * scaled;
      uplink.state.vin = real >> 1;
    } else if (input.ain == ADC_VOUT.ain) {
      int real = VOUT_MULT * scaled;
      uplink.state.vout = real >> 1;
    }
  }
};

Device device;

void interruptHandlerSERCOM0() { device.uplink.interruptHandlerSERCOM(); }
void interruptHandlerADC() { device.adc.interruptHandlerADC(); }

void initApplication() { device.init(); }
