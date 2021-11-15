#pragma once

#include <Arduino.h>
#include "controller.h"

//#define GBC_SERIAL_DEBUG            // if uncommented every update will also send a message over serial

class GasBurnerControl : public GasBurner
{
public:
    struct Settings {
        /// Number of unsuccessful dejam attempts before aborting permanently.
        uint8_t num_dejam_attempts{3};
        /// Number of unsuccessful ignition attempts before aborting permanently.
        uint8_t num_ignition_attempts{3};
        /// Initial delay in seconds after powering the GBC.
        uint8_t start_delay{2};
        /// Wait time in seconds until dejamming is possible.
        uint8_t dejam_delay_1{65};
        /// Wait time in seconds between additional dejam attempts.
        uint8_t dejam_delay_2{10};
        /// Time in seconds after which ignition should be complete.
        uint8_t ignition_duration{22};
        /// Duration of button press in milliseconds during dejamming.
        unsigned int dejam_duration{1000};
        /// Delay after dejam button release in milliseconds.
        unsigned int post_dejam_delay{1000};

        bool invert_logic_level{true};

        // TODO: replace this with a template ASAP
        int high() const;
        int low() const;

        static constexpr uint8_t max_dejam_attempts{31};
        static constexpr uint8_t max_ignition_attempts{31};
    };

    GasBurnerControl(uint8_t power_pin, uint8_t dejam_pin, uint8_t jammed_pin, uint8_t valve_pin, uint8_t ignition_pin);
    Settings settings() const;
    bool set_settings(Settings new_settings);

    void start() override;
    void stop() override;
    void update() override;
    State state() override;
    unsigned int full_state() override;

private:
    uint8_t m_power_pin;
    uint8_t m_dejam_pin;
    uint8_t m_jammed_pin;
    uint8_t m_valve_pin;
    uint8_t m_ignition_pin;

    Settings m_settings{};

    uint8_t m_ignition_counter;
    uint8_t m_dejam_counter;

    unsigned long m_start_time;
    unsigned long m_ignition_start_time;
    unsigned long m_next_dejam_attempt_time;
    unsigned long m_dejam_timer;

    State m_state;

    bool m_valve;
    bool m_jammed;
    bool m_ignition;

    void dejam(unsigned int delay_s);
};
