#include "controller.h"
#include <Arduino.h>

MainController::MainController(TemperatureSensor& brew_sensor, GasBurner& burner)
: m_brew_sensor{brew_sensor}
, m_burner{burner}
{
}

void MainController::update(unsigned long)
{
    /**
     * Brew burner flip-flop control
     */
    m_burner.update();

    const auto brew_temperature{m_brew_sensor.temperature()};
    const auto burner_state{m_burner.state()};

    if (!(m_brew_target_temperature == 0.0f)) { // act only if not in manual mode
        // safety feature: deactivate burner if temperature sensor not connected but target temperature set
        // TODO: we might wanna set a different (longer) timeout than for automatic sensor reconnects?
        if (!m_brew_sensor.is_connected() && m_brew_target_temperature != 0.0f) {
            m_burner.stop();
        }
        // TODO: We might want to check if the +-1 deg Celsius is okay here
        // TODO: maybe we want to limit switching frequency
        else if ((brew_temperature < m_brew_target_temperature - 1.0f) && (burner_state == GasBurner::State::idle)) {
            m_burner.start();
        }
        else if ((brew_temperature >= m_brew_target_temperature + 1.0f) && (burner_state != GasBurner::State::idle)) {
            m_burner.stop();
        }
    }

    }
}

void MainController::set_brew_temperature(float temperature)
{
    m_brew_target_temperature = temperature;
}

float MainController::brew_target_temperature() const
{
    return m_brew_target_temperature;
}

float MainController::brew_temperature()
{
    return m_brew_sensor.temperature();
}

bool MainController::brew_is_connected()
{
    return m_brew_sensor.is_connected();
}

bool MainController::brew_heater_is_on()
{
    return m_brew_heater_on;
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
    if (m_brew_target_temperature == 0.0f) {
        return;
    }

    if (fabs(m_brew_current_temperature - m_brew_target_temperature) < 0.001f) {
        return;
    }

    const auto grad{elapsed / 1000.0f};

    if (m_brew_current_temperature < m_brew_target_temperature) {
        m_brew_current_temperature += grad;
        m_brew_heater_on = true;
    }
    else {
        m_brew_current_temperature -= grad;
        m_brew_heater_on = false;
    }
}

void MockController::set_brew_temperature(float temperature)
{
    m_brew_target_temperature = temperature;
}

float MockController::brew_target_temperature() const
{
    return m_brew_target_temperature;
}

float MockController::brew_temperature()
{
    return m_brew_current_temperature;
}

bool MockController::brew_is_connected()
{
    return true;
}

bool MockController::brew_heater_is_on()
{
    return m_brew_heater_on;
}

bool MockController::has_problem() const
{
    return m_brew_current_temperature > 72.0f;
}

uint16_t MockController::full_burner_state()
{
    return 0;
}