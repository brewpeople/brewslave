#include "controller.h"
#include <Arduino.h>

MainController::MainController(TemperatureSensor& sensor, GasBurner& burner)
: m_sensor{sensor}
, m_burner{burner}
{
}

void MainController::update(unsigned long)
{
    m_burner.update();

    const auto temperature{m_sensor.temperature()};
    const auto burner_state{m_burner.state()};

    // safety feature: deactivate burner if temperature sensor not connected but target temperature set
    // TODO: we might wanna set a different (longer) timeout than for automatic sensor reconnects?
    if (!m_sensor.is_connected() && m_target_temperature != 0.0f) {
        m_burner.stop();
        return;
    }

    // manual mode, do nothing
    if (m_target_temperature == 0.0f) {
        return;
    }

    if ((temperature < m_target_temperature - 1.0f) && (burner_state == GasBurner::State::idle)) {
        m_burner.start();
    }
    else if ((temperature >= m_target_temperature - 1.0f) && (burner_state != GasBurner::State::idle)) {
        m_burner.stop();
    }
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

uint16_t MainController::full_burner_state()
{
    return m_burner.full_state();
}

MockController::MockController() {}

void MockController::update(unsigned long elapsed)
{
    if (m_target_temperature == 0.0f) {
        return;
    }

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

uint16_t MockController::full_burner_state()
{
    return 0;
}