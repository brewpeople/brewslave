#pragma once

#include "button.h"
#include <Arduino.h>

#define BUTTON_DEBOUNCE_TIME 100
#define BUTTON_MIN_REPEAT_TIME 1000

class PushButton : public Button {
public:
    PushButton(uint8_t);

    void trigger() final;

    void update() final;

    bool pressed() final;

private:
    uint8_t m_pin;
    uint32_t m_trigger_time{0};
    uint32_t m_last_pressed_time{0};
    State m_state{State::Idle};
};
