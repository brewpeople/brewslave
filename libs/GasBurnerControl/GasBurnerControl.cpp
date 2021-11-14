#include "GasBurnerControl.h"
#include "Arduino.h"




// GasBurnerControl::GasBurnerControl(byte powerPin, byte dejamPin, byte jammedPin, byte valvePin, byte ignitionPin, gbc_settings settings)
// : m_settings{settings}
// {
    // m_settings = settings;
    // GasBurnerControl(powerPin, dejamPin, jammedPin, valvePin, ignitionPin);
// };

GasBurnerControl::GasBurnerControl(byte powerPin, byte dejamPin, byte jammedPin, byte valvePin, byte ignitionPin, gbc_settings settings = {})
: _powerPin{powerPin}
, _dejamPin{dejamPin}
, _jammedPin{jammedPin}
, _valvePin{valvePin}
, _ignitionPin{ignitionPin}
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

// GasBurnerControl::getSettings() {
    // return m_settings;
// }

// GasBurnerControl::setSettings(gbc_settings settings) {
    // TODO: plausiblity check here?
    // m_settings = settings;
// };



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
        bool dejamRead = digitalRead(_dejamPin);
        if((dejamRead == GBC_LOW) & (_dejamTimer == 0)) {
            #ifdef GBC_SERIAL_DEBUG
            Serial.println("|---Press dejam button now");
            #endif
            digitalWrite(_dejamPin, GBC_HIGH);  // press dejam button
            _dejamTimer = millis();
        } else if(dejamRead == GBC_HIGH) {
            if(millis() - _dejamTimer >= GBC_DEJAM_DURATION) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|---Release dejam button now");
                #endif
                digitalWrite(_dejamPin, GBC_LOW);
                _dejamTimer = millis(); 
            } else {
                // wait for dejam press duration to pass
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|---Wait for dejam button release");
                #endif
            }
        } else if((dejamRead == GBC_LOW) & (_dejamTimer > 0)) {
            #ifdef GBC_SERIAL_DEBUG
            Serial.println("|---Post dejam delay");
            #endif
            if(millis() - _dejamTimer >= GBC_POST_DEJAM_DELAY) {
                // dejam should be completed, reset dejam related timers
                _dejamCounter += 1;
                _dejamTimer = 0;
                _nextDejamAttemptTime = 0;
                _state = GBC_STARTING;
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
            _state = GBC_ERROR;
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
    _state = GBC_STARTING;
    digitalWrite(_dejamPin, GBC_LOW);
    digitalWrite(_powerPin, GBC_HIGH);
}


void GasBurnerControl::stop() {
    #ifdef GBC_SERIAL_DEBUG
    Serial.println(">Stop Burner");
    #endif
    _ignitionCounter = 0;
    _dejamCounter = 0;
    _startTime = 0;
    _ignitionStartTime = 0;
    _state = GBC_IDLE;
    digitalWrite(_dejamPin, GBC_LOW);
    digitalWrite(_powerPin, GBC_LOW);
}


void GasBurnerControl::update() {
    // TODO: TESTSTUFF
    Serial.println(m_settings.startDelay);
    
    // TODO: do I have to store these as class variables? maybe not necessary (can be local)
    _valve = digitalRead(_valvePin);
    _jammed = digitalRead(_jammedPin);
    _ignition = digitalRead(_ignitionPin);
  
    switch (_state) {
        case GBC_IDLE:
            if(digitalRead(_powerPin) == GBC_LOW) {                 // state where Burner is regular off
                // pass
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">Burner regular off");
                #endif
            } else if(digitalRead(_powerPin) == GBC_HIGH) {         // state where Burner was powered on outside of class
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">Burner external on");
                #endif
                start();
            }
            break;
        
        case GBC_STARTING:                                          // state startup when Burner was powered on  
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner starting");
            #endif
            // wait some time after power on before checking the status
            if(millis() - _startTime >= GBC_START_DELAY * 1000) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Burner start delay passed");
                #endif
                if((_jammed == GBC_LOW) & ((_ignition == GBC_HIGH) | (_valve == GBC_HIGH))) {   // Burner is regular on and attempting ignition
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println(">State Change: STARTING -> IGNITION");
                    #endif
                    _state = GBC_IGNITION;
                    _ignitionStartTime = millis();
                    _ignitionCounter += 1;
                    _dejamCounter = 1;    // skips immediate dejam attempt
                } else if((_jammed == GBC_HIGH) & (_ignition == GBC_LOW) & (_valve == GBC_LOW)) {
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println(">State Change: STARTING -> DEJAM");
                    #endif
                    _state = GBC_DEJAM;
                } else {
                    #ifdef GBC_SERIAL_DEBUG
                    Serial.println(">State Change: STARTING -> ERROR");
                    #endif
                    _state = GBC_ERROR;
                }
            } else {
                // pass; do nothing until Burner is powered up
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Burner start waiting");
                #endif
            }
            break;
            
        case GBC_IGNITION:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner Ignition");
            #endif
            if(_jammed == GBC_HIGH) {                                                       // * at any time if jammed is HIGH, state change to DEJAM
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">State Change: IGNITION -> DEJAM");
                #endif
                _state = GBC_DEJAM;
            } else if(_jammed == GBC_LOW) {
                if(millis() - _ignitionStartTime >= GBC_IGNITION_DELAY * 1000) {     // * 20 s ignition valve still on --> state change to RUNNING
                    if((_jammed == GBC_LOW) & (_valve == GBC_HIGH)) {
                        _ignitionCounter = 0;
                        _dejamCounter = 1;
                        _state = GBC_RUNNING;
                    } else {
                        #ifdef GBC_SERIAL_DEBUG
                        Serial.println("|-Ignition error A -> ERROR state");
                        #endif
                        // unknown error  ---> TODO: is this realy an error? --> should i ever land here, because jam was checked at beginnig of this state
                        _state = GBC_ERROR;
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
                _state = GBC_ERROR; 
            }
            break;
        case GBC_RUNNING:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner running");
            #endif
            // continue checking for error
            if(_jammed == GBC_HIGH) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println(">State change: RUNNING -> DEJAM");
                #endif
                _state = GBC_DEJAM;
            } else {
                // pass; flame is burning
            }
            break;
        
        case GBC_DEJAM:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner dejam state");
            #endif
            // * case ignitionCounter == 0 -> first dejam attempt right away, do not increase dejamCounter
            // * case ignitionCounter > 0 -> wait 60+X s before first dejam attempt
    
            if((_ignitionCounter >= GBC_N_IGNITION_ATTEMPTS) | (_dejamCounter > GBC_N_DEJAM_ATTEMPTS)) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Exceeded ignition or dejam attempts: DEJAM -> ERROR");
                #endif
                _state = GBC_ERROR;
            } else if((_ignitionCounter == 0) & (_dejamCounter == 0)) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Dejam immediately at first start");
                #endif
                _dejam(0);
            } else if(_dejamCounter == 1) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Dejam after first delay (65ish s?)");
                #endif
                _dejam(GBC_DEJAM_DELAY);
            } else if(_dejamCounter > 1) {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Dejam secondary attempt 10s delay");
                #endif
                _dejam(GBC_DEJAM_DELAY2);
            } else {
                #ifdef GBC_SERIAL_DEBUG
                Serial.println("|-Unknown Dejam state error: DEJAM -> ERROR");
                #endif
                _state = GBC_ERROR;
            }
            break;
        
        case GBC_ERROR:
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner error state");
            #endif
            // power down but stay in this state
            digitalWrite(_powerPin, GBC_LOW);
            digitalWrite(_dejamPin, GBC_LOW);
            break;
        
        
        default:                                            // any other not defined state is changed to error state
            #ifdef GBC_SERIAL_DEBUG
            Serial.println(">Burner unknown state: ? -> ERROR");
            #endif
            _state = GBC_ERROR;
            break;

    }         // switch
} // GasBurnerControl::update()


byte GasBurnerControl::getState() {
    return _state;
}