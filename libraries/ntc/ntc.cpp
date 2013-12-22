#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "ntc.h"


NTC::NTC(uint8_t pin) : pin(pin)
{
    pinMode(pin, INPUT);
}

double NTC::temperature(void)
{
    int adc;
    double temp;

    adc = analogRead(pin);
    temp = log(((10240000. / adc) - 10000));
    temp = 1 / (0.001129148 + (0.000234125 * temp) + (0.0000000876741 * temp * temp * temp));
    return temp - 273.15;
}
