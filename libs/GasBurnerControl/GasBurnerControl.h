#pragma once

#include <Arduino.h>
#include "controller.h"


/* OPTIONS */
//#define GBC_SERIAL_DEBUG            // if uncommented every update will also send a message over serial


struct gbc_settings {
    uint8_t nDejamAttempts{3};             // number of unsuccessful dejam attempts before aborting permanently
    uint8_t nIgnitionAttempts{3};          // number of unsuccessful ignition attempts before aborting permanently

    uint8_t startDelay{2};                 // s, initial delay after powering the GBC
    uint8_t dejamDelay1{65};               // s, wait time until dejamming is possible
    uint8_t dejamDelay2{10};               // s, wait time between additional dejam attempts
    uint8_t ignitionDuration{22};          // s, time after which ignition should be complete
    unsigned int dejamDuration{1000};   // ms, duration of button press during dejamming
    unsigned int postDejamDelay{1000};  // ms, delay after dejam button release

    bool invertLogicLevel{true};

    int high() {
        return invertLogicLevel ? 0 : 1;
    };
    int low() {
        return invertLogicLevel ? 1 : 0;
    };
};

class GasBurnerControl : public GasBurner
{
public:
    GasBurnerControl(uint8_t powerPin, uint8_t dejamPin, uint8_t jammedPin, uint8_t valvePin, uint8_t ignitionPin, gbc_settings={});
    gbc_settings getSettings();
    bool setSettings(gbc_settings new_settings);

    void begin() override;
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

    gbc_settings m_settings;

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
