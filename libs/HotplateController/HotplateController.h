#pragma once

#include "hotplate.h"
#include <Arduino.h>

class HotplateController : public Hotplate {
public:
    HotplateController(uint8_t pin);

    void begin() final;
    void start() final;
    void stop() final;
    bool state() final;

private:
    const uint8_t m_pin;
};