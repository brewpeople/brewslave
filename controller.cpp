#include "controller.h"

MockController::MockController(TemperatureSensor& sensor)
: m_sensor{sensor}
{}

void MockController::set_temperature(float temperature)
{
    m_target_temperature = temperature;
}

float MockController::target_temperature() const
{
    return m_target_temperature;
}

float MockController::temperature()
{
    return m_sensor.temperature();
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
