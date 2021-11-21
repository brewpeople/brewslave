#pragma once

#include "sh1106.h"
#include "fonts.h"
#include "sensor.h"
#include "controller.h"

class Ui {
public:
    /**
     * Construct a new user interface object.
     *
     * Note, that the controller's current temperature might be read during the
     * update phase, hence make sure it is not expensive to do so.
     *
     * @param display Display used to show the UI.
     * @param controller Used to read the current set temperature.
     * @param welcome Initial welcome message.
     */
    Ui(Sh1106& display, Controller& controller, const char* welcome);

    /**
     * Update internal state and refresh display if necessary.
     */
    void update();

private:
    Sh1106& m_display;
    FontPico m_pico;
    Controller& m_controller;
    const char* m_welcome{nullptr};
    const char* m_welcome_last{nullptr};
    uint8_t m_current_scroll_start{127};
    float m_last_temperature{20.0f};
    float m_last_target_temperature{20.0f};
};
