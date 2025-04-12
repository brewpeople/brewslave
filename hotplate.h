#pragma once

#include <Arduino.h>

/**
 * Hotplate interface.
 */

class Hotplate {
public:
    /**
     * Enable hotplate controller.
     */
    virtual void begin() = 0;

    /**
     * Power up hotplate.
     */
    virtual void start() = 0;

    /**
     * Power down hotplate.
     */
    virtual void stop() = 0;

    /**
     * Return status of hotplate.
     *
     * @return true if hotplate is on.
     */
    virtual bool state() = 0;
};

class MockHotplate : public Hotplate {
public:
    void begin() final {}
    void start() final { m_state = true; }
    void stop() final { m_state = false; }
    bool state() final { return m_state; }

private:
    bool m_state{false};
};