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
     * Set target temperature.
     *
     * Set the temperature the controller should reach.
     *
     * @param temperature Target temperature in degree Celsius.
     */
    virtual void set_temperature(float temperature) = 0;

    /**
     * Get target temperature.
     */
    virtual float target_temperature() const = 0;

    /**
     * Get current temperature.
     *
     * @return Current temperature.
     */
    virtual float temperature() = 0;

    /**
     * Turn stirrer on or off.
     *
     * @param on @c true if stirrer is to be turned on, @c false if not.
     */
    virtual void set_stirrer_on(bool on) = 0;

    /**
     * Return whether stirrer is on or not.
     *
     * @return @c true if stirrer is on else @c false.
     */
    virtual bool stirrer_is_on() = 0;

    /**
     * Retrieve location of variable holding if heater is on or off.
     *
     * @return @c true if heater is on else @c false.
     */
    virtual bool heater_is_on() = 0;

    /**
     * Return @c true if there is an issue.
     */
    virtual bool has_problem() const = 0;
};

/**
 * Our main controller that tries to reach a set target temperature based on
 * temperature readings and a gas burner controll.
 */
class MainController : public Controller {
public:
    MainController(TemperatureSensor& sensor, GasBurner& burner);

    void update(unsigned long elapsed) final;

    void set_temperature(float temperature) final;

    float target_temperature() const final;

    float temperature() final;

    void set_stirrer_on(bool is_on) final;

    bool stirrer_is_on() final;

    bool heater_is_on() final;

    bool has_problem() const final;

private:
    TemperatureSensor& m_sensor;
    GasBurner& m_burner;
    float m_target_temperature{0.0f};
    bool m_stirrer_on{false};
    bool m_heater_on{false};
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

    void set_temperature(float temperature) final;

    float target_temperature() const final;

    float temperature() final;

    void set_stirrer_on(bool is_on) final;

    bool stirrer_is_on() final;

    bool heater_is_on() final;

    bool has_problem() const final;

private:
    float m_current_temperature{20.0f};
    float m_target_temperature{20.0f};
    bool m_stirrer_on{false};
    bool m_heater_on{false};
};
