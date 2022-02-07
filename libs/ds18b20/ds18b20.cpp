#include "ds18b20.h"

/**
 * Since neither DallasTemperature functions 'isConnected' nor
 * 'isConversionComplete' work in parasite mode, conversion time has to
 * be considere explicitly.
 * 9-12 bit resolution: 94, 188, 375, 750 ms conversion time
 */
#define DS18B20_RESOLUTION 10 // set desired resolution in bit (9 to 12)
#define DS18B20_CONVERSION_DURATION 188

Ds18b20::Ds18b20(uint8_t pin)
: m_wire{pin}
, m_sensors{&m_wire}
{
}

Ds18b20::Ds18b20(uint8_t pin, uint8_t pin_pullup)
: m_pin_pullup{pin_pullup}
, m_wire{pin}
, m_sensors{&m_wire}
{
    pinMode(m_pin_pullup, OUTPUT);
    set_external_pullup(true);
}

void Ds18b20::begin()
{
    reset();
}

void Ds18b20::reset()
{
    set_external_pullup(false);
    m_wire.reset();
    m_sensors.begin();
    m_sensors.getAddress(m_address, 0);
    m_sensors.setWaitForConversion(false);
    m_sensors.setResolution(DS18B20_RESOLUTION);
    m_sensors.requestTemperatures();
    set_external_pullup(true);
    const auto time{millis()};
    m_last_reconnect = time;
    m_last_interaction = time;
}

unsigned int Ds18b20::last_seen()
{
    return millis() - m_last_seen;
}

bool Ds18b20::is_connected()
{
    return !m_disconnected;
}

float Ds18b20::temperature()
{
    const auto time{millis()};
    const auto elapsed_last_seen_ms{time - m_last_seen};
    const auto elapsed_interaction_ms{time - m_last_interaction};
    const auto elapsed_reconnect_ms{time - m_last_reconnect};

    if (elapsed_last_seen_ms > 2000) { // timeout until sensor disconnect state
        m_disconnected = true;
        if (elapsed_reconnect_ms > 5000) { // try reconenct every 5s
            reset();
        }
    }
    if (elapsed_interaction_ms > DS18B20_CONVERSION_DURATION && elapsed_reconnect_ms > DS18B20_CONVERSION_DURATION) {
        m_last_interaction = time;
        set_external_pullup(false);
        int16_t raw_T = m_sensors.getTemp(m_address);
        if ((raw_T > DEVICE_DISCONNECTED_RAW) && ((raw_T >> (12 + 3 - DS18B20_RESOLUTION)) != (0x0550 >> (12 - DS18B20_RESOLUTION)))) {
            m_last_temperature = m_sensors.rawToCelsius(raw_T);
            m_last_seen = time;
            m_sensors.requestTemperatures();
            m_disconnected = false;
        }
        set_external_pullup(true);
    }
    return m_last_temperature;
}

void Ds18b20::set_external_pullup(bool state)
{
    if (m_pin_pullup == 255) {
        return;
    }
    digitalWrite(m_pin_pullup, state ? LOW : HIGH);
}