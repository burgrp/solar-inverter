# si-atsamd-configpin
Tests tri state configuration pin

Function `int atsamd::configPin::readConfigPin(int pin)` returns:
- `0` if pin is floating
- `1` if pin is pulled down
- `2` if pin is pulled hi

