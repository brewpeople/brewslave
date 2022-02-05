#pragma once

#include "burner.h"
#include <Arduino.h>

//#define GBC_SERIAL_DEBUG            // if uncommented every update will also send a message over serial

class GasBurnerControl : public GasBurner {
public:
    GasBurnerControl(uint8_t power_pin, uint8_t dejam_pin, uint8_t jammed_pin, uint8_t valve_pin, uint8_t ignition_pin);

    void begin() final;
    void start() final;
    void stop() final;
    void update() final;
    State state() final;
    unsigned int full_state() final;

private:
    uint8_t m_power_pin;
    uint8_t m_dejam_pin;
    uint8_t m_jammed_pin;
    uint8_t m_valve_pin;
    uint8_t m_ignition_pin;

    uint8_t m_ignition_counter{0};
    uint8_t m_dejam_counter{0};

    unsigned long m_start_time{0};
    unsigned long m_ignition_start_time{0};
    unsigned long m_next_dejam_attempt_time{0};
    unsigned long m_dejam_timer{0};

    State m_state{State::idle};

    bool m_valve{false};
    bool m_jammed{false};
    bool m_ignition{false};

    void dejam(unsigned int delay_s);
};
