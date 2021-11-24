#pragma once

#include "sh1106.h"
#include "fonts.h"
#include "sensor.h"
#include "controller.h"

class Updateable {
public:
    enum State : uint8_t {
        UpArrow = 1 << 0,
        DownArrow = 1 << 1,
        Warning = 1 << 2,
    };

    virtual void update() = 0;

    virtual void set_big_number(uint8_t number) = 0;

    virtual void set_small_number(uint8_t number) = 0;

    virtual void set_state(uint8_t state) = 0;
};

class Ui : public Updateable {
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
    Ui(Sh1106& display, const char* welcome);

    /**
     * Set the left-hand side number.
     */
    void set_big_number(uint8_t number) final;

    /**
     * Set the right-hand side smaller number.
     */
    void set_small_number(uint8_t number) final;

    /**
     * Set additional UI state flags.
     */
    void set_state(uint8_t) final;

    /**
     * Update internal state and refresh display if necessary.
     */
    void update() final;

private:
    Sh1106& m_display;
    FontPico m_pico;
    uint8_t m_big_number{20};
    uint8_t m_small_number{20};
    uint8_t m_state{0};
    bool m_refresh{true};
    const char* m_welcome{nullptr};
    const char* m_welcome_last{nullptr};
    uint8_t m_current_scroll_start{127};
};

class MockUi : public Updateable {
public:
    void update() final {};
    void set_big_number(uint8_t) final {};
    void set_small_number(uint8_t) final {};
    void set_state(uint8_t) final {}
};
