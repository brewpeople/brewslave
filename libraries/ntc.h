#ifndef NTC_H
#define NTC_H

class NTC {
    public:
        NTC(uint8_t pin);

        double temperature(void);

    private:
        uint8_t pin;
};

#endif
