#pragma once

#include "controller.h"
#include "display.h"
#include "fonts.h"
#include "sensor.h"

class Ui {
public:
    /**
     * Bitflags for various UI elements.
     */
    enum State : uint8_t {
        UpArrow = 1 << 0,
        DownArrow = 1 << 1,
        Warning = 1 << 2,
        SmallUpArrow = 1 << 3,
        SmallDownArrow = 1 << 4,
        SmallEq = 1 << 5,
    };

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
    Ui(Display& display, const char* welcome);

    /**
     * Set the left-hand side number.
     */
    void set_big_number(uint8_t number);

    /**
     * Set the right-hand side smaller number.
     */
    void set_small_number(uint8_t number);

    /**
     * Set additional UI state flags.
     */
    void set_state(uint8_t);

    /**
     * Update internal state and refresh display if necessary.
     *
     * @param elapsed Milliseconds elapsed since last update.
     */
    void update(unsigned long elapsed);

private:
    Display& m_display;
    FontPico m_pico;
    uint8_t m_big_number{20};
    uint8_t m_small_number{20};
    uint8_t m_state{0};
    bool m_refresh{true};
    const char* m_welcome{nullptr};
    const char* m_welcome_last{nullptr};
    uint8_t m_current_scroll_start{127};
};
