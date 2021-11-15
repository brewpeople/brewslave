#include "GasBurnerControl.h"

GasBurnerControl::GasBurnerControl(byte powerPin, byte dejamPin, byte jammedPin, byte valvePin, byte ignitionPin, gbc_settings settings)
: m_powerPin{powerPin}
, m_dejamPin{dejamPin}
, m_jammedPin{jammedPin}
, m_valvePin{valvePin}
, m_ignitionPin{ignitionPin}
, m_settings{settings}          // TODO: plausibility check similar to 'setSettings'
{
    pinMode(powerPin, OUTPUT);
    pinMode(dejamPin, OUTPUT);
    pinMode(jammedPin, INPUT);
    pinMode(valvePin, INPUT);
    pinMode(ignitionPin, INPUT);

#ifdef GBC_SERIAL_DEBUG
    Serial.println(m_settings.startDelay);
#endif

    stop();
}

gbc_settings GasBurnerControl::getSettings()
{
    return m_settings;
}

bool GasBurnerControl::setSettings(gbc_settings new_settings)
{
    // TODO: plausiblity check here? -> limit max. attempts to one digit
    if(new_settings.nDejamAttempts > 9) return false;
    if(new_settings.nIgnitionAttempts > 9) return false;
    m_settings = new_settings;
    return true;
}

void GasBurnerControl::_dejam(unsigned int delay_s)
{
    #ifdef GBC_SERIAL_DEBUG
    Serial.print("|-Burner::_dejam(");
    Serial.print(delay_s);
    Serial.println(")");
    #endif
    if(_nextDejamAttemptTime == 0) {
        _nextDejamAttemptTime = millis() + delay_s * 1000;
        #ifdef GBC_SERIAL_DEBUG
        Serial.print("|--Set next dejam attempt time to ");
        Serial.print(_nextDejamAttemptTime / 1000);
        Serial.println(" s");
        #endif
        m_state = GasBurnerControl::State::dejam_pre_delay;
    }
    if(millis() >= _nextDejamAttemptTime) {
        #ifdef GBC_SERIAL_DEBUG
        Serial.println("|--Dejamming now");
        #endif
        bool dejamRead = digitalRead(m_dejamPin);
        if((dejamRead == m_settings.low()) & (_dejamTimer == 0)) {
            #ifdef GBC_SERIAL_DEBUG
            Serial.println("|---Press dejam button now");
            #endif
            digitalWrite(m_dejamPin, m_settings.high());  // press dejam button
            _dejamTimer = millis();
            m_state = GasBurnerControl::State::dejam_button_pressed;
        } else if(dejamRead == m_settings.high()) {
            if(millis() - _dejamTimer >= m_settings.dejamDuration) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|---Release dejam button now");
                #endif
                digitalWrite(m_dejamPin, m_settings.low());
                _dejamTimer = millis();
                m_state = GasBurnerControl::State::dejam_post_delay;
            } else {
                // wait for dejam press duration to pass
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|---Wait for dejam button release");
                #endif
            }
        } else if((dejamRead == m_settings.low()) & (_dejamTimer > 0)) {
            #ifdef GBC_SERIAL_DEBUG
            Serial.println("|---Post dejam delay");
            #endif
            if(millis() - _dejamTimer >= m_settings.postDejamDelay) {
                // dejam should be completed, reset dejam related timers
                _dejamCounter += 1;
                _dejamTimer = 0;
                _nextDejamAttemptTime = 0;
                m_state = GasBurnerControl::State::starting;
                #ifdef GBC_SERIAL_DEBUG
                Serial.print("|----Dejamming attempt ");
                Serial.print(_dejamCounter);
                Serial.println(" completed");
                #endif
            } else {
                // wait until POST_DEJAM_DELAY has passed
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|----Post dejam wait");
                #endif
            }
        } else {
            #ifdef GBC_SERIAL_DEBUG
            Serial.println("|---Dejam attempt error");
            #endif
            // something went wrong
            m_state = GasBurnerControl::State::error_other;
        }
    } else {
        // do nothing
        #ifdef GBC_SERIAL_DEBUG
        Serial.println("|--Pre dejam waiting");
        #endif
    }
}

void GasBurnerControl::start()
{
    #ifdef GBC_SERIAL_DEBUG
    Serial.println(">Start Burner");
    #endif
    _ignitionCounter = 0;
    _dejamCounter = 0;
    _startTime = millis();
    _ignitionStartTime = 0;
    m_state = GasBurnerControl::State::starting;
    digitalWrite(m_dejamPin, m_settings.low());
    digitalWrite(m_powerPin, m_settings.high());
}

void GasBurnerControl::stop()
{
    #ifdef GBC_SERIAL_DEBUG
    Serial.println(">Stop Burner");
    #endif
    _ignitionCounter = 0;
    _dejamCounter = 0;
    _startTime = 0;
    _ignitionStartTime = 0;
    m_state = GasBurnerControl::State::idle;
    digitalWrite(m_dejamPin, m_settings.low());
    digitalWrite(m_powerPin, m_settings.low());
}

void GasBurnerControl::update()
{
    // TODO: do I have to store these as class variables? maybe not necessary (can be local)
    _valve = digitalRead(m_valvePin);
    _jammed = digitalRead(m_jammedPin);
    _ignition = digitalRead(m_ignitionPin);

    switch (m_state) {
        case GasBurnerControl::State::idle:
            if(digitalRead(m_powerPin) == m_settings.low()) {                 // state where Burner is regular off
                // pass
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">Burner regular off");
                #endif
            } else if(digitalRead(m_powerPin) == m_settings.high()) {         // state where Burner was powered on outside of class
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">Burner external on");
                #endif
                start();
            }
            break;

        case GasBurnerControl::State::starting:                                          // state startup when Burner was powered on
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner starting");
            #endif
            // wait some time after power on before checking the status
            if(millis() - _startTime >= m_settings.startDelay * 1000) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Burner start delay passed");
                #endif
                if((_jammed == m_settings.low()) & ((_ignition == m_settings.high()) | (_valve == m_settings.high()))) {   // Burner is regular on and attempting ignition
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println(">State Change: STARTING -> IGNITION");
                    #endif
                    _ignitionStartTime = millis();
                    _ignitionCounter += 1;
                    _dejamCounter = 1;    // skips immediate dejam attempt
                    m_state = GasBurnerControl::State::ignition;
                } else if((_jammed == m_settings.high()) & (_ignition == m_settings.low()) & (_valve == m_settings.low())) {
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println(">State Change: STARTING -> DEJAM");
                    #endif
                    m_state = GasBurnerControl::State::dejam_start;
                } else {
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println(">State Change: STARTING -> ERROR");
                    #endif
                    m_state = GasBurnerControl::State::error_other;
                }
            } else {
                // pass; do nothing until startDelay has passed
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Burner start waiting");
                #endif
            }
            break;

        case GasBurnerControl::State::ignition:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner Ignition");
            #endif
            if(_jammed == m_settings.high()) {                                                       // * at any time if jammed is HIGH, state change to DEJAM
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">State Change: IGNITION -> DEJAM");
                #endif
                m_state = GasBurnerControl::State::dejam_start;
            } else if(_jammed == m_settings.low()) {
                if(millis() - _ignitionStartTime >= m_settings.ignitionDuration * 1000) {     // * 20 s ignition valve still on --> state change to RUNNING
                    if((_jammed == m_settings.low()) & (_valve == m_settings.high())) {
                        _ignitionCounter = 0;
                        _dejamCounter = 1;
                        m_state = GasBurnerControl::State::running;
                    } else {
                        #ifdef GBC_SERIAL_DEBUG
                        Serial.println("|-Ignition error A -> ERROR state");
                        #endif
                        // this can only be reached when hardware is not working properly / wiring issue
                        m_state = GasBurnerControl::State::error_other;
                    }
                } else {
                    // ignition in progress but not yet completed
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println("|-Ignition running but not yet completed");
                    #endif
                }
            } else {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Ignition error B -> ERROR state");
                #endif
                // TODO: unused option? I should never land here because jammed indicator must be either high or low
                m_state = GasBurnerControl::State::error_other;
            }
            break;

        case GasBurnerControl::State::running:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner running");
            #endif
            // continue checking for error
            if(_jammed == m_settings.high()) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">State change: RUNNING -> DEJAM");
                #endif
                m_state = GasBurnerControl::State::dejam_start;
            } else {
                // pass; flame is burning
            }
            break;

        case GasBurnerControl::State::dejam_start:
        case GasBurnerControl::State::dejam_pre_delay:
        case GasBurnerControl::State::dejam_button_pressed:
        case GasBurnerControl::State::dejam_post_delay:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner dejam state");
            #endif
            // * case ignitionCounter == 0 -> first dejam attempt right away, do not increase dejamCounter
            // * case ignitionCounter > 0 -> wait 60+X s before first dejam attempt

            if(_ignitionCounter >= m_settings.nIgnitionAttempts) {
                // exceeded max. ignition attepts
                m_state = GasBurnerControl::State::error_ignition;
            } else if((_ignitionCounter == 0) & (_dejamCounter > m_settings.nDejamAttempts)) {
                // exceeded max. dejam attempts
                if(_ignitionCounter == 0) {
                    // ignition never started (dejamming at start unsuccessful) -> wiring issue? // power supply issue?
                    m_state = GasBurnerControl::State::error_start;
                } else {
                    // implies that dejam attempts were exceeded, ignition was at least once but did not reach its max. attempts --> should never happen? wiring?
                    m_state = GasBurnerControl::State::error_other;
                }
            } else if((_ignitionCounter == 0) & (_dejamCounter == 0)) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Dejam immediately at first start");
                #endif
                _dejam(0);
            } else if(_dejamCounter == 1) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Dejam after first delay (65ish s?)");
                #endif
                _dejam(m_settings.dejamDelay1);
            } else if(_dejamCounter > 1) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Dejam secondary attempt 10s delay");
                #endif
                _dejam(m_settings.dejamDelay2);
            } else {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Unknown Dejam state error: DEJAM -> ERROR");
                #endif
                // should never be reached unless coding with ignition or dejam counter is faulty
                // can only be reached when dejamCounter == 0 and ignitionCounter > 0
                m_state = GasBurnerControl::State::error_other;
            }
            break;

        case GasBurnerControl::State::error_start:
        case GasBurnerControl::State::error_ignition:
        case GasBurnerControl::State::error_other:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner error state");
            #endif
            // power down but stay in this state
            digitalWrite(m_powerPin, m_settings.low());
            digitalWrite(m_dejamPin, m_settings.low());
            break;

        default:                                            // any other not defined state is changed to error state, TODO: do i need this with enum class states?
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner unknown state: ? -> ERROR");
            #endif
            m_state = GasBurnerControl::State::error_other;
            break;

    }         // switch
} // GasBurnerControl::update()

GasBurnerControl::State GasBurnerControl::getState()
{
    return m_state;
}

unsigned int GasBurnerControl::getFullState()
{
    return (unsigned int) m_state * 100 + _ignitionCounter * 10 + _dejamCounter;
}
