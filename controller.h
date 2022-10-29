#pragma once

#include "burner.h"
#include "sensor.h"
#include <Arduino.h>

/**
 * Main controller interface reading temperatures and trying to set the
 * temperature in a control loop based on a burner control.
 *
 * A target temperature == 0 deactivates the controller.
 */
class Controller {
public:
    /**
     * Update internal state and potential set variables.
     *
     * @param elapsed Number of milliseconds elapsed since last call.
     */
    virtual void update(unsigned long elapsed) = 0;

    /**
     * Set brew target temperature.
     *
     * Set the temperature the brew controller should reach.
     *
     * @param temperature Target temperature in degree Celsius.
     */
    virtual void set_brew_temperature(float temperature) = 0;

    /**
     * Get brew target temperature.
     */
    virtual float brew_target_temperature() const = 0;

    /**
     * Get current brew temperature.
     *
     * @return Current brew temperature.
     */
    virtual float brew_temperature() = 0;

    /**
     * Check if brew temperature sensor is connected.
     *
     * @return bool.
     */
    virtual bool brew_is_connected() = 0;

    /**
     * Retrieve location of variable holding if heater is on or off.
     *
     * @return @c true if heater is on else @c false.
     */
    virtual bool brew_heater_is_on() = 0;

    /**
     * Return @c true if there is an issue.
     */
    virtual bool has_problem() const = 0;

    /**
     * Expose full burner state.
     */
    virtual uint16_t full_burner_state() = 0;
};

/**
 * Our main controller that tries to reach a set target temperature based on
 * temperature readings and a gas burner controll.
 */
class MainController : public Controller {
public:
    MainController(TemperatureSensor& brew_sensor, GasBurner& burner);

    void update(unsigned long elapsed) final;

    void set_brew_temperature(float temperature) final;

    float brew_target_temperature() const final;

    float brew_temperature() final;

    bool brew_is_connected() final;

    bool brew_heater_is_on() final;

    bool has_problem() const final;

    uint16_t full_burner_state() final;

private:
    TemperatureSensor& m_brew_sensor;
    GasBurner& m_burner;
    float m_brew_target_temperature{0.0f};
    bool m_brew_heater_on{false};
};

/**
 * A software mock controller that does not access any real hardware.
 *
 * A heating curve is implemented that approaches the set target temperature
 * with a rate of +/- 1 degree Celsius per second.
 */
class MockController : public Controller {
public:
    MockController();

    void update(unsigned long elapsed) final;

    void set_brew_temperature(float temperature) final;

    float brew_target_temperature() const final;

    float brew_temperature() final;

    bool brew_is_connected() final;

    bool brew_heater_is_on() final;

    bool has_problem() const final;

    uint16_t full_burner_state() final;

private:
    float m_brew_current_temperature{20.0f};
    float m_brew_target_temperature{0.0f};
    bool m_brew_heater_on{false};
};
