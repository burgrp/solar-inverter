class Controller {
public:
  Uplink *uplink;
  PWM *pwm;

  int vin_mV;
  int vout_mV;
  int duty;
  int maxDuty;

  int target_mV = 20000;

  void init(Uplink *uplink, PWM *pwm) {
    this->uplink = uplink;
    this->pwm = pwm;

    maxDuty = pwm->fullDuty / 2;
    uplink->state.maxDuty = maxDuty;

    update();
  }

  void setVIN(int mV) {
    vin_mV = mV;
    update();
  }

  void setVOUT(int mV) {
    vout_mV = mV;
    update();
  }

  void update() {
    
    if (vout_mV < target_mV) {
        duty++;
    } else {
        duty--;
    }

    if (duty < 0) {
        duty = 0;
    } else if (duty > maxDuty) {
        duty = maxDuty;
    }

    pwm->set(duty);

    uplink->state.vin_mV = vin_mV;
    uplink->state.vout_mV = vout_mV;
    uplink->state.actDuty = duty;
  }
};