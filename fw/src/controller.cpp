class Controller {
public:
  Uplink *uplink;
  PWM *pwm;

  int vCap_mV;
  int duty;
  int maxDuty;

  int target_mV = 24000;

  void init(Uplink *uplink, PWM *pwm) {
    this->uplink = uplink;
    this->pwm = pwm;

    maxDuty = pwm->fullDuty;
    uplink->state.maxDuty = maxDuty;

    update();
  }

  void setVCap(int mV) {
    vCap_mV = mV;
    update();
  }

  int lastError_mV = 0;

  void update() {
    
    int error_mV = vCap_mV - target_mV + lastError_mV;
    lastError_mV = error_mV;

    // const int maxError_mV = 100;
    // if (error_mV > maxError_mV) {
    //   error_mV = maxError_mV;
    // } else if (error_mV < -maxError_mV) {
    //   error_mV = -maxError_mV;
    // }   
    duty = duty - error_mV >> 15;

    // if (vout_mV < target_mV) {
    //     duty+=1;
    // } else {
    //     duty-=1;
    // }

    if (duty < 0) {
        duty = 0;
    } else if (duty > maxDuty) {
        duty = maxDuty;
    }

    pwm->set(duty);

    uplink->state.vCap_mV = vCap_mV;
    uplink->state.actDuty = duty;
  }
};