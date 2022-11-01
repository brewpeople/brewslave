#pragma once

#include "controller.h"
#include "display.h"
#include "fonts.h"
#include "sensor.h"

class Ui {
public:
    /**
     * States for multiple UI layouts.
     *
     */
    enum Layout : uint8_t {
        LayoutA = 1,
        LayoutB = 2,
    };

    /**
     * Bitflags for various UI elements.
     */
    enum State : uint8_t {
        UpArrowA = 1 << 0,
        DownArrowA = 1 << 1,
        UpArrowB = 1 << 2,
        DownArrowB = 1 << 3,
        SmallUpArrow = 1 << 4,
        SmallDownArrow = 1 << 5,
        SmallEq = 1 << 6,
        InduOn = 1 << 7,
        // Warning = 1 << 7, //INFO: Keep for now, until we are confident that gbc controller works and we decide to go back to simple/clean UI
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
     * Enable/disable layout switching.
     */
    void set_layout_switching(bool enabled);

    /**
     * Freeze current layout.
     *
     * @return Current layout.
     */
    Layout freeze_layout(bool freeze);

    /**
     * Get active layout.
     */
    Layout current_layout();

    /**
     * Set current layout.
     */
    void set_layout(Layout layout);

    /**
     * Set the big number.
     * LayoutA: left
     * LayoutB: right
     */
    void set_big_number_a(uint8_t number);
    void set_big_number_b(uint8_t number);

    /**
     * Set the smaller number.
     * LayoutA: right
     * LayoutB: left
     */
    void set_small_number_a(uint8_t number);
    void set_small_number_b(uint8_t number);

    /**
     * Set additional UI state flags.
     */
    void set_state(uint8_t);

    /**
     * Set GBC full state.
     */
    void set_full_burner_state(uint16_t);

    /**
     * Update internal state and refresh display if necessary.
     */
    void update();

private:
    Display& m_display;
    FontPico m_pico;
    bool m_layout_switching{false};
    bool m_freeze_layout{false};
    Layout m_current_layout{LayoutA};
    unsigned long m_last_layout_switch{0};
    uint8_t m_big_number_a{20};
    uint8_t m_small_number_a{20};
    uint8_t m_big_number_b{20};
    uint8_t m_small_number_b{20};
    uint8_t m_state{0};
    uint16_t m_full_burner_state{0};
    bool m_refresh{true};
    unsigned long m_last_update{0};
    const char* m_welcome{nullptr};
    const char* m_welcome_last{nullptr};
    uint8_t m_current_scroll_start{127};
};
