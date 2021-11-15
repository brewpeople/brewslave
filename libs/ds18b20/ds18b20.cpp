#include "ds18b20.h"

Ds18b20::Ds18b20(uint8_t pin)
: m_wire{pin}
, m_sensors{&m_wire}
{}

float Ds18b20::temperature()
{
    m_sensors.requestTemperatures();
    return m_sensors.getTempCByIndex(0);
}
