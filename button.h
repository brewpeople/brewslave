#pragma once

class Button {
public:
    enum class State {
        Idle,
        Unconfirmed,
        Pressed,
    };

    /**
     * Wrapper to be called from pin interrupt.
     */
    virtual void trigger() = 0;

    /**
     * Updates the status of the button. Must be called repeatedly during the
     * main loop.
     * Checks if a minimum time of 1000 ms passed since last successfull button press,
     * and if an unconfirmed interrupt is a valid button press after 50 ms debounce.
     */
    virtual void update() = 0;

    /**
     * Returns true if a valid button press occured.
     */
    virtual bool pressed() = 0;
};

class MockButton : public Button {
public:
    // void begin() final {}
    void update() final {}
    void trigger() final {}
    bool pressed() final { return false; }
};
