#pragma once

#include <Arduino.h>

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
        error_dejam = 12,
        error_other = 13,
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
    uint16_t full_state() final;

private:
    GasBurner::State m_state;
    uint8_t m_ignition_counter;
    uint8_t m_dejam_counter;
    unsigned long m_last_state_change_time{0};
};
