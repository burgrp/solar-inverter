const clkFreqHz = 8000000;
const pwmTimerFreqHz = 120000;
const pwmTimerDivider = 1;
const acTimerFreqHz = 50;
const acTimerDivider = 256;

const pwmTimerSteps = Math.floor(clkFreqHz / pwmTimerDivider / pwmTimerFreqHz);
const acTimerSteps = Math.floor(clkFreqHz / acTimerDivider / acTimerFreqHz / 4) * 4;

console.info(`namespace generated {

const int CLK_FREQ_HZ = ${clkFreqHz};
const int PWM_TIMER_STEPS = ${pwmTimerSteps};
const target::tcc::CTRLA::PRESCALER PWM_TIMER_DIVIDER = target::tcc::CTRLA::PRESCALER::DIV${pwmTimerDivider};
const int AC_TIMER_STEPS = ${acTimerSteps};
const target::tc::COUNT32::CTRLA::PRESCALER AC_TIMER_DIVIDER = target::tc::COUNT32::CTRLA::PRESCALER::DIV${acTimerDivider};

// values to be multiplied by level <0..100%> represented as <0..256> and shifted right 24 bits
__attribute__((section(".text")))
const int QUARTER_AC_CC[AC_TIMER_STEPS / 4] = {`);
for (let i = 0; i < acTimerSteps / 4; i++) {
    let pos = i / (acTimerSteps / 2 - 2);
    let sin = Math.sin(Math.PI * pos);
    let val = Math.round(0x10000 * sin * pwmTimerSteps);
    let last = i < acTimerSteps / 4 - 1;
    console.info(`${val}${last && "," || ""}`);
        
}

console.info(`};

}`);
