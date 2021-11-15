#include "GasBurnerControl.h"

int GasBurnerControl::Settings::high() const
{
    return invert_logic_level ? 0 : 1;
}

int GasBurnerControl::Settings::low() const
{
    return invert_logic_level ? 1 : 0;
}

GasBurnerControl::GasBurnerControl(uint8_t power_pin, uint8_t dejam_pin, uint8_t jammed_pin, uint8_t valve_pin, uint8_t ignition_pin)
: m_power_pin{power_pin}
, m_dejam_pin{dejam_pin}
, m_jammed_pin{jammed_pin}
, m_valve_pin{valve_pin}
, m_ignition_pin{ignition_pin}
{
    pinMode(power_pin, OUTPUT);
    pinMode(dejam_pin, OUTPUT);
    pinMode(jammed_pin, INPUT);
    pinMode(valve_pin, INPUT);
    pinMode(ignition_pin, INPUT);

#ifdef GBC_SERIAL_DEBUG
    Serial.println(m_settings.start_delay);
#endif

    stop();
}

GasBurnerControl::Settings GasBurnerControl::settings() const
{
    return m_settings;
}

bool GasBurnerControl::set_settings(GasBurnerControl::Settings new_settings)
{
    // TODO: plausiblity check here? -> limit max. attempts to one digit
    // TODO: why make this a run-time decision? There is no way to react anyway.
    if (new_settings.num_dejam_attempts > 9)
        return false;

    if (new_settings.num_ignition_attempts > 9)
        return false;

    m_settings = new_settings;
    return true;
}

void GasBurnerControl::dejam(unsigned int delay_s)
{
    #ifdef GBC_SERIAL_DEBUG
    Serial.print("|-Burner::_dejam(");
    Serial.print(delay_s);
    Serial.println(")");
    #endif
    if (m_next_dejam_attempt_time == 0) {
        m_next_dejam_attempt_time = millis() + delay_s * 1000;
        #ifdef GBC_SERIAL_DEBUG
        Serial.print("|--Set next dejam attempt time to ");
        Serial.print(m_next_dejam_attempt_time / 1000);
        Serial.println(" s");
        #endif
        m_state = GasBurner::State::dejam_pre_delay;
    }

    if (millis() >= m_next_dejam_attempt_time) {
        #ifdef GBC_SERIAL_DEBUG
        Serial.println("|--Dejamming now");
        #endif
        bool dejamRead = digitalRead(m_dejam_pin);
        if ((dejamRead == m_settings.low()) & (m_dejam_timer == 0)) {
            #ifdef GBC_SERIAL_DEBUG
            Serial.println("|---Press dejam button now");
            #endif
            digitalWrite(m_dejam_pin, m_settings.high());  // press dejam button
            m_dejam_timer = millis();
            m_state = GasBurner::State::dejam_button_pressed;
        }
        else if (dejamRead == m_settings.high()) {
            if(millis() - m_dejam_timer >= m_settings.dejam_duration) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|---Release dejam button now");
                #endif
                digitalWrite(m_dejam_pin, m_settings.low());
                m_dejam_timer = millis();
                m_state = GasBurner::State::dejam_post_delay;
            }
            else {
                // wait for dejam press duration to pass
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|---Wait for dejam button release");
                #endif
            }
        }
        else if ((dejamRead == m_settings.low()) & (m_dejam_timer > 0)) {
            #ifdef GBC_SERIAL_DEBUG
            Serial.println("|---Post dejam delay");
            #endif
            if (millis() - m_dejam_timer >= m_settings.post_dejam_delay) {
                // dejam should be completed, reset dejam related timers
                m_dejam_counter += 1;
                m_dejam_timer = 0;
                m_next_dejam_attempt_time = 0;
                m_state = GasBurner::State::starting;
                #ifdef GBC_SERIAL_DEBUG
                Serial.print("|----Dejamming attempt ");
                Serial.print(m_dejam_counter);
                Serial.println(" completed");
                #endif
            }
            else {
                // wait until POST_DEJAM_DELAY has passed
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|----Post dejam wait");
                #endif
            }
        }
        else {
            #ifdef GBC_SERIAL_DEBUG
            Serial.println("|---Dejam attempt error");
            #endif
            // something went wrong
            m_state = GasBurner::State::error_other;
        }
    }
    else {
        // do nothing
        #ifdef GBC_SERIAL_DEBUG
        Serial.println("|--Pre dejam waiting");
        #endif
    }
}

void GasBurnerControl::begin()
{
    #ifdef GBC_SERIAL_DEBUG
    Serial.println(">Start Burner");
    #endif
    m_ignition_counter = 0;
    m_dejam_counter = 0;
    m_start_time = millis();
    m_ignition_start_time = 0;
    m_state = GasBurner::State::starting;
    digitalWrite(m_dejam_pin, m_settings.low());
    digitalWrite(m_power_pin, m_settings.high());
}

void GasBurnerControl::stop()
{
    #ifdef GBC_SERIAL_DEBUG
    Serial.println(">Stop Burner");
    #endif
    m_ignition_counter = 0;
    m_dejam_counter = 0;
    m_start_time = 0;
    m_ignition_start_time = 0;
    m_state = GasBurner::State::idle;
    digitalWrite(m_dejam_pin, m_settings.low());
    digitalWrite(m_power_pin, m_settings.low());
}

void GasBurnerControl::update()
{
    // TODO: do I have to store these as class variables? maybe not necessary (can be local)
    m_valve = digitalRead(m_valve_pin);
    m_jammed = digitalRead(m_jammed_pin);
    m_ignition = digitalRead(m_ignition_pin);

    switch (m_state) {
        case GasBurner::State::idle:
            if (digitalRead(m_power_pin) == m_settings.low()) {                 // state where Burner is regular off
                // pass
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">Burner regular off");
                #endif
            }
            else if (digitalRead(m_power_pin) == m_settings.high()) {         // state where Burner was powered on outside of class
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">Burner external on");
                #endif
                begin();
            }
            break;

        case GasBurner::State::starting:                                          // state startup when Burner was powered on
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner starting");
            #endif
            // wait some time after power on before checking the status
            if (millis() - m_start_time >= m_settings.start_delay * 1000) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Burner start delay passed");
                #endif
                if ((m_jammed == m_settings.low()) & ((m_ignition == m_settings.high()) | (m_valve == m_settings.high()))) {   // Burner is regular on and attempting ignition
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println(">State Change: STARTING -> IGNITION");
                    #endif
                    m_ignition_start_time = millis();
                    m_ignition_counter += 1;
                    m_dejam_counter = 1;    // skips immediate dejam attempt
                    m_state = GasBurner::State::ignition;
                }
                else if ((m_jammed == m_settings.high()) & (m_ignition == m_settings.low()) & (m_valve == m_settings.low())) {
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println(">State Change: STARTING -> DEJAM");
                    #endif
                    m_state = GasBurner::State::dejam_start;
                }
                else {
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println(">State Change: STARTING -> ERROR");
                    #endif
                    m_state = GasBurner::State::error_other;
                }
            }
            else {
                // pass; do nothing until start_delay has passed
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Burner start waiting");
                #endif
            }
            break;

        case GasBurner::State::ignition:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner Ignition");
            #endif
            if (m_jammed == m_settings.high()) {                                                       // * at any time if jammed is HIGH, state change to DEJAM
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">State Change: IGNITION -> DEJAM");
                #endif
                m_state = GasBurner::State::dejam_start;
            }
            else if (m_jammed == m_settings.low()) {
                if (millis() - m_ignition_start_time >= m_settings.ignition_duration * 1000) {     // * 20 s ignition valve still on --> state change to RUNNING
                    if ((m_jammed == m_settings.low()) & (m_valve == m_settings.high())) {
                        m_ignition_counter = 0;
                        m_dejam_counter = 1;
                        m_state = GasBurner::State::running;
                    }
                    else {
                        #ifdef GBC_SERIAL_DEBUG
                        Serial.println("|-Ignition error A -> ERROR state");
                        #endif
                        // this can only be reached when hardware is not working properly / wiring issue
                        m_state = GasBurner::State::error_other;
                    }
                }
                else {
                    // ignition in progress but not yet completed
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println("|-Ignition running but not yet completed");
                    #endif
                }
            }
            else {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Ignition error B -> ERROR state");
                #endif
                // TODO: unused option? I should never land here because jammed indicator must be either high or low
                m_state = GasBurner::State::error_other;
            }
            break;

        case GasBurner::State::running:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner running");
            #endif
            // continue checking for error
            if (m_jammed == m_settings.high()) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">State change: RUNNING -> DEJAM");
                #endif
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
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner dejam state");
            #endif
            // * case ignitionCounter == 0 -> first dejam attempt right away, do not increase dejamCounter
            // * case ignitionCounter > 0 -> wait 60+X s before first dejam attempt

            if (m_ignition_counter >= m_settings.num_ignition_attempts) {
                // exceeded max. ignition attepts
                m_state = GasBurner::State::error_ignition;
            }
            else if ((m_ignition_counter == 0) & (m_dejam_counter > m_settings.num_dejam_attempts)) {
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
            else if((m_ignition_counter == 0) & (m_dejam_counter == 0)) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Dejam immediately at first start");
                #endif
                dejam(0);
            }
            else if(m_dejam_counter == 1) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Dejam after first delay (65ish s?)");
                #endif
                dejam(m_settings.dejam_delay_1);
            }
            else if(m_dejam_counter > 1) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Dejam secondary attempt 10s delay");
                #endif
                dejam(m_settings.dejam_delay_2);
            }
            else {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Unknown Dejam state error: DEJAM -> ERROR");
                #endif
                // should never be reached unless coding with ignition or dejam counter is faulty
                // can only be reached when dejamCounter == 0 and ignitionCounter > 0
                m_state = GasBurner::State::error_other;
            }
            break;

        case GasBurner::State::error_start:
        case GasBurner::State::error_ignition:
        case GasBurner::State::error_other:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner error state");
            #endif
            // power down but stay in this state
            digitalWrite(m_power_pin, m_settings.low());
            digitalWrite(m_dejam_pin, m_settings.low());
            break;

        default:                                            // any other not defined state is changed to error state, TODO: do i need this with enum class states?
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner unknown state: ? -> ERROR");
            #endif
            m_state = GasBurner::State::error_other;
            break;
    }         // switch
} // GasBurnerControl::update()

GasBurner::State GasBurnerControl::state()
{
    return m_state;
}

unsigned int GasBurnerControl::full_state()
{
    return (unsigned int) m_state * 100 + m_ignition_counter * 10 + m_dejam_counter;
}
