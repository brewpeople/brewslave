#include "GasBurnerControl.h"


GasBurnerControl::GasBurnerControl(byte powerPin, byte dejamPin, byte jammedPin, byte valvePin, byte ignitionPin, gbc_settings settings)
: m_powerPin{powerPin}
, m_dejamPin{dejamPin}
, m_jammedPin{jammedPin}
, m_valvePin{valvePin}
, m_ignitionPin{ignitionPin}
, m_settings{settings}
{
    pinMode(powerPin, OUTPUT);
    pinMode(dejamPin, OUTPUT);
    pinMode(jammedPin, INPUT);
    pinMode(valvePin, INPUT);
    pinMode(ignitionPin, INPUT);
    
    Serial.println(m_settings.startDelay);

    stop();
}


gbc_settings GasBurnerControl::getSettings() {
    return m_settings;
}

void GasBurnerControl::setSettings(gbc_settings new_settings) {
    // TODO: plausiblity check here?
    m_settings = new_settings;
}



void GasBurnerControl::_dejam(unsigned int delay_s) {
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
        } else if(dejamRead == m_settings.high()) {
            if(millis() - _dejamTimer >= m_settings.dejamDuration) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|---Release dejam button now");
                #endif
                digitalWrite(m_dejamPin, m_settings.low());
                _dejamTimer = millis(); 
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
                m_state = GasBurnerControlState::starting;
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
            m_state = GasBurnerControlState::error;
        }
    } else {
        // do nothing
        #ifdef GBC_SERIAL_DEBUG
        Serial.println("|--Pre dejam waiting");
        #endif
    }
}


void GasBurnerControl::start() {
    #ifdef GBC_SERIAL_DEBUG
    Serial.println(">Start Burner");
    #endif
    _ignitionCounter = 0;
    _dejamCounter = 0;
    _startTime = millis();
    _ignitionStartTime = 0;
    m_state = GasBurnerControlState::starting;
    digitalWrite(m_dejamPin, m_settings.low());
    digitalWrite(m_powerPin, m_settings.high());
}


void GasBurnerControl::stop() {
    #ifdef GBC_SERIAL_DEBUG
    Serial.println(">Stop Burner");
    #endif
    _ignitionCounter = 0;
    _dejamCounter = 0;
    _startTime = 0;
    _ignitionStartTime = 0;
    m_state = GasBurnerControlState::idle;
    digitalWrite(m_dejamPin, m_settings.low());
    digitalWrite(m_powerPin, m_settings.low());
}


void GasBurnerControl::update() {
    // TODO: do I have to store these as class variables? maybe not necessary (can be local)
    _valve = digitalRead(m_valvePin);
    _jammed = digitalRead(m_jammedPin);
    _ignition = digitalRead(m_ignitionPin);
  
    switch (m_state) {
        case GasBurnerControlState::idle:
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
        
        case GasBurnerControlState::starting:                                          // state startup when Burner was powered on  
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
                    m_state = GasBurnerControlState::ignition;
                } else if((_jammed == m_settings.high()) & (_ignition == m_settings.low()) & (_valve == m_settings.low())) {
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println(">State Change: STARTING -> DEJAM");
                    #endif
                    m_state = GasBurnerControlState::dejam;
                } else {
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println(">State Change: STARTING -> ERROR");
                    #endif
                    m_state = GasBurnerControlState::error;
                }
            } else {
                // pass; do nothing until Burner is powered up
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Burner start waiting");
                #endif
            }
            break;
            
        case GasBurnerControlState::ignition:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner Ignition");
            #endif
            if(_jammed == m_settings.high()) {                                                       // * at any time if jammed is HIGH, state change to DEJAM
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">State Change: IGNITION -> DEJAM");
                #endif
                m_state = GasBurnerControlState::dejam;
            } else if(_jammed == m_settings.low()) {
                if(millis() - _ignitionStartTime >= m_settings.ignitionDuration * 1000) {     // * 20 s ignition valve still on --> state change to RUNNING
                    if((_jammed == m_settings.low()) & (_valve == m_settings.high())) {
                        _ignitionCounter = 0;
                        _dejamCounter = 1;
                        m_state = GasBurnerControlState::running;
                        
                    } else {
                        #ifdef GBC_SERIAL_DEBUG
                        Serial.println("|-Ignition error A -> ERROR state");
                        #endif
                        // unknown error  ---> TODO: is this realy an error? --> should i ever land here, because jam was checked at beginnig of this state
                        m_state = GasBurnerControlState::error;
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
                // unknown error
                m_state = GasBurnerControlState::error; 
            }
            break;

        case GasBurnerControlState::running:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner running");
            #endif
            // continue checking for error
            if(_jammed == m_settings.high()) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">State change: RUNNING -> DEJAM");
                #endif
                m_state = GasBurnerControlState::dejam;
            } else {
                // pass; flame is burning
            }
            break;
        
        case GasBurnerControlState::dejam:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner dejam state");
            #endif
            // * case ignitionCounter == 0 -> first dejam attempt right away, do not increase dejamCounter
            // * case ignitionCounter > 0 -> wait 60+X s before first dejam attempt
    
            if((_ignitionCounter >= m_settings.nIgnitionAttempts) | (_dejamCounter > m_settings.nDejamAttempts)) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Exceeded ignition or dejam attempts: DEJAM -> ERROR");
                #endif
                m_state = GasBurnerControlState::error;
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
                m_state = GasBurnerControlState::error;
            }
            break;
        
        case GasBurnerControlState::error:
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
            m_state = GasBurnerControlState::error;
            break;

    }         // switch
} // GasBurnerControl::update()


GasBurnerControlState GasBurnerControl::getState() {
    return m_state;
}