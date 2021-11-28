#include <Arduino.h>
#include "controller.h"

MainController::MainController(TemperatureSensor& sensor)
: m_sensor{sensor}
{}

void MainController::update()
{
}

void MainController::set_temperature(float temperature)
{
    m_target_temperature = temperature;
}

float MainController::target_temperature() const
{
    return m_target_temperature;
}

float MainController::temperature()
{
    return m_sensor.temperature();
}

void MainController::set_stirrer_on(bool is_on)
{
    m_stirrer_on = is_on;
}

bool MainController::stirrer_is_on()
{
    return m_stirrer_on;
}

bool MainController::heater_is_on()
{
    return m_heater_on;
}

bool MainController::has_problem() const
{
    return false;
}

MockController::MockController()
: m_last_time{millis()}
{}

void MockController::update()
{
    if (fabs(m_current_temperature - m_target_temperature) < 0.001f) {
        return;
    }

    const auto now{millis()};
    const auto grad{(now - m_last_time) / 1000.0f};

    if (m_current_temperature < m_target_temperature) {
        m_current_temperature += grad;
        m_heater_on = true;
    }
    else {
        m_current_temperature -= grad;
        m_heater_on = false;
    }

    m_last_time = now;
}

void MockController::set_temperature(float temperature)
{
    m_target_temperature = temperature;
    m_last_time = millis();
}

float MockController::target_temperature() const
{
    return m_target_temperature;
}

float MockController::temperature()
{
    return m_current_temperature;
}

void MockController::set_stirrer_on(bool is_on)
{
    m_stirrer_on = is_on;
}

bool MockController::stirrer_is_on()
{
    return m_stirrer_on;
}

bool MockController::heater_is_on()
{
    return m_heater_on;
}

bool MockController::has_problem() const
{
    return m_current_temperature > 72.0f;
}
