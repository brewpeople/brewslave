#pragma once

#include "config.h"

/**
 * Temperature sensor interface.
 */
class TemperatureSensor {
public:
    virtual void begin();

    /**
     * Read the current temperature.
     */
    virtual float temperature() = 0;

    /**
     * Returns the elapsed time in ms since the last successful sensor reading.
     */
    virtual unsigned int elapsed() = 0;

    /**
     * Return true if last sensor reading was successful. Indicates if the value
     * provided by 'temperature()' was successfully updated in the last cycle.
     * If false, the return temperature value is old.
     */
    virtual bool is_connected() = 0;
};

class MockTemperatureSensor : public TemperatureSensor {
public:
    void begin() final {}

    float temperature() final { return 20.0f; }

    unsigned int elapsed() final { return 0; }

    bool is_connected() final { return true; }
};
