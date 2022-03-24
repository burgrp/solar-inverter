namespace atsamd::configPin {

/**
 * @brief Test tri state configuration pin
 *  * 
 * @param pin 
 * @return int 0 if pin is floating, 1 if pin is pulled down, 2 if pin is pulled hi
 */
int readConfigPin(int pin) {

  target::PORT.DIRCLR = 1 << pin;
  target::PORT.PINCFG[pin].setINEN(true).setPULLEN(true);

  target::PORT.OUTSET = 1 << pin;
  int puValue = (target::PORT.IN >> pin) & 1;

  target::PORT.OUTCLR = 1 << pin;
  int pdValue = (target::PORT.IN >> pin) & 1;

  target::PORT.PINCFG[pin].setINEN(false).setPULLEN(false);

  return (puValue && pdValue) ? 2 : (!puValue && !pdValue) ? 1 : 0;
}

} // namespace atsamd::configPin