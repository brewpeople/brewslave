#include "controller.h"

void MockController::set_temperature(float temperature)
{
    m_temperature = temperature;
}

float MockController::temperature()
{
    return m_temperature;
}

void MockController::set_stirrer_on(bool is_on)
{
    m_stirrer_on = is_on;
}

bool MockController::stirrer_is_on()
{
    return m_stirrer_on;
}

void MockController::set_heater_on(bool is_on)
{
    m_heater_on = is_on;
}

bool MockController::heater_is_on()
{
    return m_heater_on;
}
