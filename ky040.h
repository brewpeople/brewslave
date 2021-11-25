#pragma once

#include "encoder.h"
#include "RotaryEncoder.h"

class Ky040 : public ButtonEncoder {
public:
    Ky040(uint8_t sw, uint8_t dt, uint8_t clk);

    void update() final;

    ButtonEncoder::Direction direction() final;

    bool pressed() final;

private:
    void pin_change(uint8_t pin);

    uint8_t m_sw{0};
    uint8_t m_sw_pin;
    bool m_pressed{false};
    RotaryEncoder m_encoder;
};
