#include "ds18b20.h"

#define DS18B20_RESOLUTION 10 // set desired resolution in bit (9 to 12)

/**
 * Since neither DallasTemperature functions 'isConnected' nor
 * 'isConversionComplete' work in parasite mode, conversion time has to
 * be considere explicitly.
 * 9-12 bit resolution: 94, 188, 375, 750 ms conversion time
 */
#if DS18B20_RESOLUTION == 9
#define DS18B20_CONVERSION_DURATION 94
#elif DS18B20_RESOLUTION == 10
#define DS18B20_CONVERSION_DURATION 188
#elif DS18B20_RESOLUTION == 11
#define DS18B20_CONVERSION_DURATION 375
#elif DS18B20_RESOLUTION == 12
#define DS18B20_CONVERSION_DURATION 750
#endif

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
    set_external_pullup(false);
    m_sensors.begin();
    m_sensors.getAddress(m_address, 0);
    reset();
    set_external_pullup(true);
}

void Ds18b20::reset()
{
    set_external_pullup(false);
    if (m_sensors.isConnected(m_address)) {
        m_sensors.setWaitForConversion(false);
        m_sensors.setResolution(DS18B20_RESOLUTION);
        m_sensors.requestTemperatures();
        m_last_seen = millis();
    }
    else {
        m_disconnected = true;
        m_wire.reset();
        m_sensors.begin();
        m_sensors.getAddress(m_address, 0);
    }
    set_external_pullup(true);
}

unsigned int Ds18b20::elapsed()
{
    return millis() - m_last_seen;
}

bool Ds18b20::is_connected()
{
    return !m_disconnected;
}

float Ds18b20::temperature()
{
    const auto elapsed_ms{elapsed()};

    if (elapsed_ms > 5000) { // timeout for sensor connection
        m_disconnected = true;
        if (elapsed_ms / 1000 % 5 == 0) { // try reconenct every 5s (at the moment this tries for 1s every 5s, is there a better approach attempting this only once without additional flag?)
            reset();
        }
    }
    if (elapsed_ms > DS18B20_CONVERSION_DURATION) {
        set_external_pullup(false);

        /**
         * The 'not connected' value is defined in the DallasTemperature lib as
         * -127 in C or -7040 (DEVICE_DISCONNECTED_RAW) in raw.
         * If DS18B20 resolution is below maximum, the omitted lower bits may be
         * undefined/floating. I am not sure whether the power/conversion
         * error (+85 C, 0x0550) is also affected by this. Hence, the raw value is
         * read and shifted, omitting the lower undefined/floating bits.
         *
         * This results in the following readings regarded as invalid:
         *  9 bit: +85.0000 to +85.4375
         * 10 bit: +85.0000 to +85.1875
         * 11 bit: +85.0000 to +85.0625
         * 12 bit: +85.0000
         */
        int16_t raw_T = m_sensors.getTemp(m_address);
        if ((raw_T > DEVICE_DISCONNECTED_RAW) && ((raw_T >> (12 + 3 - DS18B20_RESOLUTION)) != (0x0550 >> (12 - DS18B20_RESOLUTION)))) {
            m_last_temperature = m_sensors.rawToCelsius(raw_T);
            m_last_seen = millis();
            m_sensors.requestTemperaturesByAddress(m_address);
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