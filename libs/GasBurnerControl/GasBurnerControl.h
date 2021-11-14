#ifndef GasBurnerControl_h
#define GasBurnerControl_h


#include <Arduino.h>


/* OPTIONS */
//#define GBC_SERIAL_DEBUG            // if uncommented every update will also send a message over serial


struct gbc_settings {
    byte nDejamAttempts{3};             // number of unsuccessful dejam attempts before aborting permanently
    byte nIgnitionAttempts{3};          // number of unsuccessful ignition attempts before aborting permanently

    byte startDelay{2};                 // s, initial delay after powering the GBC
    byte dejamDelay1{65};               // s, wait time until dejamming is possible
    byte dejamDelay2{10};               // s, wait time between additional dejam attempts
    byte ignitionDuration{22};          // s, time after which ignition should be complete
    unsigned int dejamDuration{1000};   // ms, duration of button press during dejamming
    unsigned int postDejamDelay{1000};  // ms, delay after dejam button release
    
    bool invertLogicLevel{true};
    
    int high() {
        return invertLogicLevel ? 0 : 1;
    };
    int low() {
        return invertLogicLevel ? 1 : 0;
    };
};


enum class GasBurnerControlState {
    idle = 0,
    starting = 10,
    ignition = 20,
    running = 30,
    any_dejam = 4,
    dejam_start = 41,
    dejam_pre_delay = 42,
    dejam_button_pressed = 43,
    dejam_post_delay = 44,
    any_error = 5,
    error_start = 51,
    error_ignition = 52,
    error_other = 53
};


#ifdef WITH_GBC
class GasBurnerControl
{
    public:
        GasBurnerControl(byte powerPin, byte dejamPin, byte jammedPin, byte valvePin, byte ignitionPin, gbc_settings={});
        // GasBurnerControl(byte powerPin, byte dejamPin, byte jammedPin, byte valvePin, byte ignitionPin);
        gbc_settings getSettings();
        bool setSettings(gbc_settings new_settings);
        void start();
        void stop();
        void update();
        GasBurnerControlState getState();
        unsigned int getFullState();


    private:
        byte m_powerPin;
        byte m_dejamPin;
        byte m_jammedPin;
        byte m_valvePin;
        byte m_ignitionPin;

        gbc_settings m_settings;

        byte _ignitionCounter;
        byte _dejamCounter;

        unsigned long _startTime;
        unsigned long _ignitionStartTime;
        unsigned long _nextDejamAttemptTime;
        unsigned long _dejamTimer;

        GasBurnerControlState m_state;

        bool _valve;
        bool _jammed;
        bool _ignition;

        void _dejam(unsigned int delay_s);
};


#else
class GasBurnerControl{
    public:
        GasBurnerControl(byte powerPin, byte dejamPin, byte jammedPin, byte valvePin, byte ignitionPin, gbc_settings={}) {};
        void start() {};
        void stop() {};
        void update() {
            if(millis() - _lastStateChangeTime >= 2000) {
                _lastStateChangeTime = millis();
                byte mock_main_state = rand() % 6;
                byte mock_substate = 0;
                if(mock_main_state == 4) {
                    mock_substate = rand() % 4 + 1;
                } else if(mock_main_state == 5) {
                    mock_substate = rand() % 3 + 1;
                }
                m_state = static_cast<GasBurnerControlState> (mock_main_state * 10 + mock_substate);
                if(mock_main_state > 0) {
                    _ignitionCounter = rand() % 4;
                    _dejamCounter = rand() % 4;
                } else {
                    _ignitionCounter = 0;
                    _dejamCounter = 0;
                }
            }
        };
        GasBurnerControlState getState() {
             return m_state;
        };
        unsigned int getFullState() {
            return (unsigned int) m_state * 100 + _ignitionCounter * 10 + _dejamCounter;
        };
    private:
        GasBurnerControlState m_state;
        byte _ignitionCounter;
        byte _dejamCounter;
        unsigned long _lastStateChangeTime{0};
};
#endif // #ifdef WITH_GBC

#endif // #ifnedf GasBurnerControl_h