#include "GasBurnerControl.h"

#ifdef GBC_SERIAL_DEBUG
#define GBC_DEBUG(x) Serial.print((x))
#define GBC_DEBUGLN(x) Serial.println((x))
#else
#define GBC_DEBUG(x)
#define GBC_DEBUGLN(x)
#endif

namespace gbc {
    /// Number of unsuccessful dejam attempts before aborting permanently.
    constexpr uint8_t num_dejam_attempts{3};
    /// Number of unsuccessful ignition attempts before aborting permanently.
    constexpr uint8_t num_ignition_attempts{3};
    /// Initial delay in seconds after powering the GBC.
    constexpr uint8_t start_delay{2};
    /// Wait time in seconds until dejamming is possible.
    constexpr uint8_t dejam_delay_1{65};
    /// Wait time in seconds between additional dejam attempts.
    constexpr uint8_t dejam_delay_2{10};
    /// Time in seconds after which ignition should be complete.
    constexpr uint8_t ignition_duration{22};
    /// Duration of button press in milliseconds during dejamming.
    constexpr uint16_t dejam_duration{1000};
    /// Delay after dejam button release in milliseconds.
    constexpr uint16_t post_dejam_delay{1000};

    constexpr int high{1};
    constexpr int low{0};

    constexpr uint8_t max_dejam_attempts{31};
    constexpr uint8_t max_ignition_attempts{31};
}

static_assert((gbc::high == 1 && gbc::low == 0) || (gbc::high == 0 && gbc::low == 1), "high/low must mutually exlusive be either 1 or 0");
static_assert(gbc::num_dejam_attempts < gbc::max_dejam_attempts, "Cannot have more than 31 dejam attempts");
static_assert(gbc::num_ignition_attempts < gbc::max_ignition_attempts, "Cannot have more than 31 ignition attempts");

GasBurnerControl::GasBurnerControl(uint8_t power_pin, uint8_t dejam_pin, uint8_t jammed_pin, uint8_t valve_pin, uint8_t ignition_pin)
: m_power_pin{power_pin}
, m_dejam_pin{dejam_pin}
, m_jammed_pin{jammed_pin}
, m_valve_pin{valve_pin}
, m_ignition_pin{ignition_pin}
{
}

void GasBurnerControl::begin()
{
    pinMode(m_power_pin, OUTPUT);
    pinMode(m_dejam_pin, OUTPUT);
    pinMode(m_jammed_pin, INPUT);
    pinMode(m_valve_pin, INPUT);
    pinMode(m_ignition_pin, INPUT);

    GBC_DEBUGLN(gbc::start_delay);

    stop();
}

void GasBurnerControl::dejam(unsigned int delay_s)
{
    GBC_DEBUG(F("|-Burner::_dejam("));
    GBC_DEBUG(delay_s);
    GBC_DEBUG(F("   "));
    GBC_DEBUG(m_dejam_counter);
    GBC_DEBUGLN(F(")"));

    if (m_next_dejam_attempt_time == 0) {
        m_next_dejam_attempt_time = millis() + delay_s * 1000;
        GBC_DEBUG(F("|--Set next dejam attempt time to "));
        GBC_DEBUG(m_next_dejam_attempt_time / 1000);
        GBC_DEBUGLN(F(" s"));
        m_state = GasBurner::State::dejam_pre_delay;
    }

    if (millis() >= m_next_dejam_attempt_time) {
        GBC_DEBUGLN(F("|--Dejamming now"));
        bool dejamRead = digitalRead(m_dejam_pin);
        if ((dejamRead == gbc::low) & (m_dejam_timer == 0)) {
            GBC_DEBUGLN(F("|---Press dejam button now"));
            digitalWrite(m_dejam_pin, gbc::high); // press dejam button
            m_dejam_timer = millis();
            m_state = GasBurner::State::dejam_button_pressed;
        }
        else if (dejamRead == gbc::high) {
            if (millis() - m_dejam_timer >= gbc::dejam_duration) {
                GBC_DEBUGLN(F("|---Release dejam button now"));
                digitalWrite(m_dejam_pin, gbc::low);
                m_dejam_timer = millis();
                m_state = GasBurner::State::dejam_post_delay;
            }
            else {
                // wait for dejam press duration to pass
                GBC_DEBUGLN(F("|---Wait for dejam button release"));
            }
        }
        else if ((dejamRead == gbc::low) & (m_dejam_timer > 0)) {
            GBC_DEBUGLN(F("|---Post dejam delay"));
            if (millis() - m_dejam_timer >= gbc::post_dejam_delay) {
                // dejam should be completed, reset dejam related timers
                m_dejam_counter += 1;
                m_dejam_timer = 0;
                m_next_dejam_attempt_time = 0;
                m_state = GasBurner::State::starting;
                GBC_DEBUG(F("|----Dejamming attempt "));
                GBC_DEBUG(m_dejam_counter);
                GBC_DEBUGLN(F(" completed"));
            }
            else {
                // wait until POST_DEJAM_DELAY has passed
                GBC_DEBUGLN(F("|----Post dejam wait"));
            }
        }
        else {
            GBC_DEBUGLN(F("|---Dejam attempt error"));
            // something went wrong
            m_state = GasBurner::State::error_other;
        }
    }
    else {
        // do nothing
        GBC_DEBUGLN(F("|--Pre dejam waiting"));
    }
}

void GasBurnerControl::start()
{
    GBC_DEBUGLN(F(">Start Burner"));
    m_ignition_counter = 0;
    m_dejam_counter = 0;
    m_start_time = millis();
    m_ignition_start_time = 0;
    m_state = GasBurner::State::starting;
    digitalWrite(m_dejam_pin, gbc::low);
    digitalWrite(m_power_pin, gbc::high);
}

void GasBurnerControl::stop()
{
    GBC_DEBUGLN(F(">Stop Burner"));
    m_ignition_counter = 0;
    m_dejam_counter = 0;
    m_start_time = 0;
    m_ignition_start_time = 0;
    m_state = GasBurner::State::idle;
    digitalWrite(m_dejam_pin, gbc::low);
    digitalWrite(m_power_pin, gbc::low);
}

void GasBurnerControl::update()
{
    // TODO: do I have to store these as class variables? maybe not necessary (can be local)
    m_valve = digitalRead(m_valve_pin);
    m_jammed = digitalRead(m_jammed_pin);
    m_ignition = digitalRead(m_ignition_pin);

    switch (m_state) {
        case GasBurner::State::idle:
            if (digitalRead(m_power_pin) == gbc::low) { // state where Burner is regular off
                // pass
                GBC_DEBUGLN(F(">Burner regular off"));
            }
            else if (digitalRead(m_power_pin) == gbc::high) { // state where Burner was powered on outside of class
                GBC_DEBUGLN(F(">Burner external on"));
                start();
            }
            break;

        case GasBurner::State::starting: // state startup when Burner was powered on
            GBC_DEBUGLN(F(">Burner starting"));
            // wait some time after power on before checking the status
            if (millis() - m_start_time >= gbc::start_delay * 1000) {
                GBC_DEBUGLN(F("|-Burner start delay passed"));
                if ((m_jammed == gbc::low) & ((m_ignition == gbc::high) | (m_valve == gbc::high))) { // Burner is regular on and attempting ignition
                    GBC_DEBUGLN(F(">State Change: STARTING -> IGNITION"));
                    m_ignition_start_time = millis();
                    m_ignition_counter += 1;
                    m_dejam_counter = 1; // skips immediate dejam attempt
                    m_state = GasBurner::State::ignition;
                }
                else if ((m_jammed == gbc::high) & (m_ignition == gbc::low) & (m_valve == gbc::low)) {
                    GBC_DEBUGLN(F(">State Change: STARTING -> DEJAM"));
                    m_state = GasBurner::State::dejam_start;
                }
                else {
                    GBC_DEBUGLN(F(">State Change: STARTING -> ERROR"));
                    m_state = GasBurner::State::error_other;
                }
            }
            else {
                // pass; do nothing until start_delay has passed
                GBC_DEBUGLN(F("|-Burner start waiting"));
            }
            break;

        case GasBurner::State::ignition:
            GBC_DEBUGLN(F(">Burner Ignition"));
            if (m_jammed == gbc::high) { // * at any time if jammed is HIGH, state change to DEJAM
                GBC_DEBUGLN(F(">State Change: IGNITION -> DEJAM"));
                m_state = GasBurner::State::dejam_start;
            }
            else if (m_jammed == gbc::low) {
                if (millis() - m_ignition_start_time >= gbc::ignition_duration * 1000) { // * 20 s ignition valve still on --> state change to RUNNING
                    if ((m_jammed == gbc::low) & (m_valve == gbc::high)) {
                        m_ignition_counter = 0;
                        m_dejam_counter = 1;
                        m_state = GasBurner::State::running;
                    }
                    else {
                        GBC_DEBUGLN(F("|-Ignition error A -> ERROR state"));
                        // this can only be reached when hardware is not working properly / wiring issue
                        m_state = GasBurner::State::error_other;
                    }
                }
                else {
                    // ignition in progress but not yet completed
                    GBC_DEBUGLN(F("|-Ignition running but not yet completed"));
                }
            }
            else {
                GBC_DEBUGLN(F("|-Ignition error B -> ERROR state"));
                // TODO: unused option? I should never land here because jammed indicator must be either high or low
                m_state = GasBurner::State::error_other;
            }
            break;

        case GasBurner::State::running:
            GBC_DEBUGLN(F(">Burner running"));
            // continue checking for error
            if (m_jammed == gbc::high) {
                GBC_DEBUGLN(F(">State change: RUNNING -> DEJAM"));
                m_state = GasBurner::State::dejam_start;
            }
            else {
                // pass; flame is burning
            }
            break;

        case GasBurner::State::dejam_start:
        case GasBurner::State::dejam_pre_delay:
        case GasBurner::State::dejam_button_pressed:
        case GasBurner::State::dejam_post_delay:
            GBC_DEBUGLN(F(">Burner dejam state"));
            // * case ignitionCounter == 0 -> first dejam attempt right away, do not increase dejamCounter
            // * case ignitionCounter > 0 -> wait 60+X s before first dejam attempt

            if (m_ignition_counter >= gbc::num_ignition_attempts) {
                // exceeded max. ignition attepts
                m_state = GasBurner::State::error_ignition;
            }
            else if ((m_ignition_counter == 0) & (m_dejam_counter > gbc::num_dejam_attempts)) {
                // exceeded max. dejam attempts
                if (m_ignition_counter == 0) {
                    // ignition never started (dejamming at start unsuccessful) -> wiring issue? // power supply issue?
                    m_state = GasBurner::State::error_start;
                }
                else {
                    // implies that dejam attempts were exceeded, ignition was at least once but did not reach its max. attempts --> should never happen? wiring?
                    m_state = GasBurner::State::error_other;
                }
            }
            else if ((m_ignition_counter == 0) & (m_dejam_counter == 0)) {
                GBC_DEBUGLN(F("|-Dejam immediately at first start"));
                dejam(0);
            }
            else if (m_dejam_counter == 1) {
                GBC_DEBUGLN(F("|-Dejam after first delay (65ish s?)"));
                dejam(gbc::dejam_delay_1);
            }
            else if (m_dejam_counter > 1) {
                GBC_DEBUGLN(F("|-Dejam secondary attempt 10s delay"));
                dejam(gbc::dejam_delay_2);
            }
            else {
                GBC_DEBUGLN(F("|-Unknown Dejam state error: DEJAM -> ERROR"));
                // should never be reached unless coding with ignition or dejam counter is faulty
                // can only be reached when dejamCounter == 0 and ignitionCounter > 0
                m_state = GasBurner::State::error_other;
            }
            break;

        case GasBurner::State::error_start:
        case GasBurner::State::error_ignition:
        case GasBurner::State::error_other:
            GBC_DEBUGLN(F(">Burner error state"));
            // power down but stay in this state
            digitalWrite(m_power_pin, gbc::low);
            digitalWrite(m_dejam_pin, gbc::low);
            break;

        default: // any other not defined state is changed to error state, TODO: do i need this with enum class states?
            GBC_DEBUGLN(F(">Burner unknown state: ? -> ERROR"));
            m_state = GasBurner::State::error_other;
            break;
    } // switch
} // GasBurnerControl::update()

GasBurner::State GasBurnerControl::state()
{
    return m_state;
}

uint16_t GasBurnerControl::full_state()
{
    return GasBurner::encode_state(m_state, m_dejam_counter, m_ignition_counter);
}
