const clkFreqHz = 8000000;
const pwmTimerFreqHz = 120000;
const pwmTimerPrescaler = 1;
const acTimerFreqHz = 50;
const acTimerPrescaler = 1024;

const pwmTimerSteps = Math.floor(clkFreqHz / pwmTimerPrescaler / pwmTimerFreqHz);
const acTimerSteps = Math.floor(clkFreqHz / acTimerPrescaler / acTimerFreqHz / 2);

console.info(`namespace generated {

const int CLK_FREQ_HZ = ${clkFreqHz};
const int PWM_TIMER_STEPS = ${pwmTimerSteps};
const target::tcc::CTRLA::PRESCALER PWM_TIMER_PRESCALER = target::tcc::CTRLA::PRESCALER::DIV${pwmTimerPrescaler};
const int AC_TIMER_STEPS = ${acTimerSteps};
const target::tc::COUNT16::CTRLA::PRESCALER AC_TIMER_PRESCALER = target::tc::COUNT16::CTRLA::PRESCALER::DIV${acTimerPrescaler};

// values to be multiplied by level <0..100%> represented as <0..256> and shifted right 24 bits
__attribute__((section(".text")))
const int AC_SINE[AC_TIMER_STEPS] = {`);
for (let i = 0; i < acTimerSteps; i++) {
    let pos = i / acTimerSteps;
    let sin = Math.sin(Math.PI * pos);
    let val = Math.round(0x10000 * sin * pwmTimerSteps);
    let last = i < acTimerSteps - 1;
    console.info(`${val}${last && "," || ""} // ${pos}: ${sin}`);
        
}

console.info(`};

}`);
