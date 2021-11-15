#pragma once

#include "config.h"

/**
 * Temperature sensor interface.
 */
class TemperatureSensor {
public:
    /**
     * Read the current temperature.
     */
    virtual float temperature() = 0;
};

class MockTemperatureSensor : public TemperatureSensor {
public:
    float temperature() final { return 20.0f; }
};
