#ifndef NTC_H
#define NTC_H

#include <inttypes.h>

class NTC {
    public:
        NTC(uint8_t pin);

        double temperature(void);

    private:
        uint8_t pin;
};

#endif
