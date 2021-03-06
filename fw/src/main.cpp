const int LED_PIN = 8;
const int SAFE_BOOT_PIN = 9;

const int SLAVE_SDA_PIN = 14;
const int SLAVE_SCL_PIN = 15;
const target::port::PMUX::PMUXE SLAVE_SDA_MUX = target::port::PMUX::PMUXE::C;
const target::port::PMUX::PMUXE SLAVE_SCL_MUX = target::port::PMUX::PMUXE::C;

const int GATE_PIN = 5;
const target::port::PMUX::PMUXE GATE_MUX = target::port::PMUX::PMUXE::E;
const int GATE_WO_INDEX = 1;

const int VC_R1 = 10000;
const int VC_R2 = 270;
const int VC_MULT = (VC_R1 + VC_R2) / VC_R2;
const ADC::Input ADC_VC = {pin : 2, ain : 0};

const ADC::Input ADC_INPUTS[1] = {ADC_VC};

class Device : public ADC::Callback {
public:
  Uplink uplink;
  PWM pwm;
  ADC adc;
  Controller controller;

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

    genericTimer::clkHz = 48E6;

    // debug - output generator 0 to PA24
    // target::PORT.PMUX[24 >> 1].setPMUXE(target::port::PMUX::PMUXE::H);
    // target::PORT.PINCFG[24].setPMUXEN(true).setDRVSTR(true);

    // initialize subsystems

    adc.init(ADC_INPUTS, sizeof(ADC_INPUTS) / sizeof(ADC::Input), this);

    uplink.init(8, &target::SERCOM0, target::gclk::CLKCTRL::GEN::GCLK0,
                SLAVE_SDA_PIN, SLAVE_SDA_MUX, SLAVE_SCL_PIN, SLAVE_SCL_MUX);

    pwm.init(&target::TC1, target::gclk::CLKCTRL::GEN::GCLK0, GATE_PIN,
             GATE_MUX, GATE_WO_INDEX);

    controller.init(&uplink, &pwm);

    // enable interrupts

    target::NVIC.IPR[target::interrupts::External::SERCOM0 >> 2].setPRI(
        target::interrupts::External::SERCOM0 & 0x03, 3);

    target::NVIC.ISER.setSETENA(1 << target::interrupts::External::SERCOM0);
    target::NVIC.ISER.setSETENA(1 << target::interrupts::External::ADC);
  }

  void adcRead(ADC::Input input, int scaled) {
    if (input.ain == ADC_VC.ain) {
      controller.setVCap(VC_MULT * scaled);
    }
  }
};

Device device;

void interruptHandlerSERCOM0() { device.uplink.interruptHandlerSERCOM(); }
void interruptHandlerADC() { device.adc.interruptHandlerADC(); }

void initApplication() { device.init(); }
