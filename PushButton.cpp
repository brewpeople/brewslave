#include "PushButton.h"

PushButton::PushButton(uint8_t pin)
: m_pin{pin}
{
}

void PushButton::trigger()
{
    uint32_t time{millis()};
    if (m_state == State::Idle && time - m_last_pressed_time >= BUTTON_MIN_REPEAT_TIME) {
        m_state = State::Unconfirmed;
        m_trigger_time = time;
    }
}

void PushButton::update()
{
    uint32_t time{millis()};
    if (m_state == State::Unconfirmed && (time - m_trigger_time) >= BUTTON_DEBOUNCE_TIME) {
        if (digitalRead(m_pin) == HIGH) {
            m_state = State::Pressed;
            m_last_pressed_time = time;
        }
        else {
            m_state = State::Idle;
        }
    }
}

bool PushButton::pressed()
{
    if (m_state == State::Pressed) {
        m_state = State::Idle;
        return true;
    }
    else {
        return false;
    }
}