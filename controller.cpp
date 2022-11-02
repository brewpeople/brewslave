#include "controller.h"
#include <Arduino.h>

MainController::MainController(TemperatureSensor& brew_sensor, TemperatureSensor& sparging_sensor, GasBurner& burner, Hotplate& hotplate)
: m_brew_sensor{brew_sensor}
, m_sparging_sensor{sparging_sensor}
, m_burner{burner}
, m_hotplate{hotplate}
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

    /**
     * Sparging hotplate flip-flop controller
     */

    const auto sparging_temperature{m_sparging_sensor.temperature()};

    if (!(m_sparging_target_temperature == 0.0f)) { // act only if not in manual mode
        // safety feature: deactivate hotplate if temperature sensor not connected but target temperature set
        // TODO: we might wanna set a different (longer) timeout than for automatic sensor reconnects?
        if (!m_sparging_sensor.is_connected() && m_sparging_target_temperature != 0.0f) {
            m_hotplate.stop();
        }
        // TODO: We might want to check if the +-1 deg Celsius is okay here
        // TODO: maybe we want to limit switching frequency
        else if (sparging_temperature < m_sparging_target_temperature - 1.0f) {
            m_hotplate.start();
        }
        else if (sparging_temperature >= m_sparging_target_temperature + 1.0f) {
            m_hotplate.stop();
        }
    }
}

void MainController::set_brew_temperature(float temperature)
{
    m_brew_target_temperature = temperature;
}

void MainController::set_sparging_temperature(float temperature)
{
    m_sparging_target_temperature = temperature;
}

float MainController::brew_target_temperature() const
{
    return m_brew_target_temperature;
}

float MainController::sparging_target_temperature() const
{
    return m_sparging_target_temperature;
}

float MainController::brew_temperature()
{
    return m_brew_sensor.temperature();
}

float MainController::sparging_temperature()
{
    return m_sparging_sensor.temperature();
}

bool MainController::brew_is_connected()
{
    return m_brew_sensor.is_connected();
}

bool MainController::sparging_is_connected()
{
    return m_sparging_sensor.is_connected();
}

bool MainController::brew_heater_is_on()
{
    if (m_burner.state() == GasBurner::State::running) {
        return true;
    }
    else {
        return false;
    }
}

bool MainController::sparging_heater_is_on()
{
    return m_hotplate.state();
}

bool MainController::has_problem() const
{
    return false;
}

GasBurner::State MainController::burner_state()
{
    return m_burner.state();
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

void MockController::set_sparging_temperature(float temperature)
{
    m_sparging_target_temperature = temperature;
}

float MockController::brew_target_temperature() const
{
    return m_brew_target_temperature;
}

float MockController::sparging_target_temperature() const
{
    return m_sparging_target_temperature;
}

float MockController::brew_temperature()
{
    return m_brew_current_temperature;
}

float MockController::sparging_temperature()
{
    return m_sparging_current_temperature;
}

bool MockController::brew_is_connected()
{
    return true;
}

bool MockController::sparging_is_connected()
{
    return true;
}

bool MockController::brew_heater_is_on()
{
    return m_brew_heater_on;
}
bool MockController::sparging_heater_is_on()
{
    return m_sparging_heater_on;
}

bool MockController::has_problem() const
{
    return m_brew_current_temperature > 72.0f;
}

GasBurner::State MockController::burner_state()
{
    if (m_brew_heater_on == true) {
        return GasBurner::State::running;
    }
    else {
        return GasBurner::State::idle;
    }
}

uint16_t MockController::full_burner_state()
{
    return 0;
}