#pragma once

/* Deactivate the alarm function of DallasTemperature library */
#define REQUIRESALARMS false

#include "sensor.h"
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>


class Ds18b20 : public TemperatureSensor {
public:
    Ds18b20(uint8_t pin);

    /**
     * For parasite power, an external pullup on the data line in the form
     * of a P-MOSFET is recommended in the datasheet (Figure 6).
     */
    Ds18b20(uint8_t pin, uint8_t pin_pullup);

    void begin() final;
    float temperature() final;
    unsigned int elapsed() final;
    bool is_connected() final;

private:
    uint8_t m_pin_pullup{255};          // lacking a better "not defined" state
    OneWire m_wire;
    DallasTemperature m_sensors;
    DeviceAddress m_address;            // default value required?
    float m_last_temperature{20.0f};
    unsigned long m_last_seen{0};
    bool m_disconnected{true};

    void reset();
    void set_external_pullup(bool state);
};
