#include "ky040.h"

Ky040::Ky040(uint8_t sw, uint8_t dt, uint8_t clk)
: m_sw_pin{sw}
, m_encoder{dt, clk}
{
    pinMode(sw, INPUT_PULLUP);

    pin_change(sw);
    pin_change(dt);
    pin_change(clk);
}

void Ky040::update()
{
    m_sw = digitalRead(m_sw_pin);
    m_encoder.tick();
}

ButtonEncoder::Direction Ky040::direction()
{
    switch (m_encoder.getDirection()) {
        case RotaryEncoder::Direction::CLOCKWISE:
            return ButtonEncoder::Direction::Clockwise;
        case RotaryEncoder::Direction::COUNTERCLOCKWISE:
            return ButtonEncoder::Direction::CounterClockwise;
        case RotaryEncoder::Direction::NOROTATION:
            return ButtonEncoder::Direction::None;
    };

    return ButtonEncoder::Direction::None;
}

bool Ky040::pressed()
{
    const auto pressed{m_sw == 1};
    m_sw = 0;
    return pressed;
}

void Ky040::pin_change(uint8_t pin)
{
    *digitalPinToPCMSK(pin) |= bit(digitalPinToPCMSKbit(pin));
    PCIFR |= bit(digitalPinToPCICRbit(pin));
    PCICR |= bit(digitalPinToPCICRbit(pin));
}
