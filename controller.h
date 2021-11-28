#pragma once

#include <Arduino.h>
#include "sensor.h"

/**
 * State-machine based gas burner interface.
 */
class GasBurner {
public:
    enum class State : uint8_t {
        idle = 0,
        // matze: should we really expose starting and ignition to the user of
        // the class? AFAICS at that layer we are only interested in if it's
        // running or not and then start/stop it if its not or is.
        starting = 1,
        ignition = 2,
        running = 3,
        any_dejam = 4,
        dejam_start = 5,
        dejam_pre_delay = 6,
        dejam_button_pressed = 7,
        dejam_post_delay = 8,
        any_error = 9,
        error_start = 10,
        error_ignition = 11,
        error_other = 12,
        // XXX: we use six bits for the state in order to encode dejam and
        // ignition counts in the remaining ten bits, so never add a state
        // larger than 63.
    };

    /**
     * Encode state and counter in a 16 bit value.
     *
     * @param state State encoded in the lower six bits.
     * @param dejam_counter Counter encoded in the upper five bits.
     * @param ignition_counter Counter encoded in the middle five bits.
     */
    static uint16_t encode_state(State state, uint8_t dejam_counter, uint8_t ignition_counter);

    /**
     * One-time initialization to be called in setup().
     */
    virtual void begin() = 0;

    /**
     * Start burner.
     */
    virtual void start() = 0;

    /**
     * Stop burner.
     */
    virtual void stop() = 0;

    /**
     * Update states based on external pins.
     */
    virtual void update() = 0;

    /**
     * Get current state.
     */
    virtual State state() = 0;

    /**
     * Get current full state.
     *
     * @return Full state composed of 5 bits (maximum 31) for dejam count, 5
     *  bits for ignition and 6 bits for State.
     */
    virtual uint16_t full_state() = 0;
};

class MockGasBurner : public GasBurner {
public:
    void begin() final;
    void start() final;
    void stop() final;
    void update() final;
    State state() final;
    unsigned int full_state() final;

private:
    GasBurner::State m_state;
    uint8_t m_ignition_counter;
    uint8_t m_dejam_counter;
    unsigned long m_last_state_change_time{0};
};

/**
 * Main controller interface reading temperatures and trying to set the
 * temperature in a control loop based on a burner control.
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
    float m_target_temperature{20.0f};
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
