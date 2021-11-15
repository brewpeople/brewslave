#include <Arduino.h>
#include "controller.h"

MainController::MainController(TemperatureSensor& sensor)
: m_sensor{sensor}
{}

void MainController::update(unsigned long)
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
{}

void MockController::update(unsigned long elapsed)
{
    if (fabs(m_current_temperature - m_target_temperature) < 0.001f) {
        return;
    }

    const auto grad{elapsed / 1000.0f};

    if (m_current_temperature < m_target_temperature) {
        m_current_temperature += grad;
        m_heater_on = true;
    }
    else {
        m_current_temperature -= grad;
        m_heater_on = false;
    }
}

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

void MockGasBurner::start()
{
}

void MockGasBurner::stop()
{
}

void MockGasBurner::update()
{
    const auto now{millis()};

    if (now - m_last_state_change_time >= 2000) {
        m_last_state_change_time = now;
        uint8_t mock_main_state = rand() % 6;
        uint8_t mock_substate = 0;

        if (mock_main_state == 4) {
            mock_substate = rand() % 4 + 1;
        }
        else if (mock_main_state == 5) {
            mock_substate = rand() % 3 + 1;
        }

        m_state = static_cast<GasBurner::State> (mock_main_state * 10 + mock_substate);

        if (mock_main_state > 0) {
            m_ignition_counter = rand() % 4;
            m_dejam_counter = rand() % 4;
        }
        else {
            m_ignition_counter = 0;
            m_dejam_counter = 0;
        }
    }
}

GasBurner::State MockGasBurner::state()
{
    return m_state;
}

unsigned int MockGasBurner::full_state()
{
    return (unsigned int) m_state * 100 + m_ignition_counter * 10 + m_dejam_counter;
}
