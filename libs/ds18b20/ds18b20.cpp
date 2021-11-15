#include "ds18b20.h"

Ds18b20::Ds18b20(uint8_t pin)
: m_wire{pin}
, m_sensors{&m_wire}
{
    m_sensors.setWaitForConversion(false);
    m_sensors.requestTemperaturesByIndex(0);
}

float Ds18b20::temperature()
{
    if (m_sensors.isConversionComplete()) {
        m_last_temperature = m_sensors.getTempCByIndex(0);
        m_sensors.requestTemperaturesByIndex(0);
    }

    return m_last_temperature;
}
