#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "sensor.h"

class Ds18b20 : public TemperatureSensor {
public:
    Ds18b20(uint8_t pin);
    float temperature() final;

private:
    OneWire m_wire;
    DallasTemperature m_sensors;
    float m_last_temperature{20.0f};
};
