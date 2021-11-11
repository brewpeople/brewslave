#pragma once

#include "config.h"

#ifdef WITH_DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

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

#ifdef WITH_DS18B20
class Ds18b20 : public TemperatureSensor {
public:
    Ds18b20(uint8_t pin);
    float temperature() final;

private:
    OneWire m_wire;
    DallasTemperature m_sensors;
};
#endif
