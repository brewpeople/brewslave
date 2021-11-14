#ifndef GasBurnerControl_h
#define GasBurnerControl_h


#include <Arduino.h>


/* OPTIONS */
#define GBC_HIGH 0                  // logical high (0: GND, 1: VCC)
#define GBC_LOW 1                   // logical low (0: GND, 1: VCC)

#define GBC_SERIAL_DEBUG            // if uncommented every update will also send a message over serial


/* STATE REPRESENTATIONS (must be unique) */
#define GBC_IDLE 0
#define GBC_STARTING 1
#define GBC_IGNITION 2
#define GBC_RUNNING 3
#define GBC_DEJAM 4
#define GBC_ERROR 5


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
    starting = 1,
    ignition = 2,
    running = 3,
    dejam = 4,
    error = 5
};


class GasBurnerControl
{
    public:
        GasBurnerControl(byte powerPin, byte dejamPin, byte jammedPin, byte valvePin, byte ignitionPin, gbc_settings={});
        // GasBurnerControl(byte powerPin, byte dejamPin, byte jammedPin, byte valvePin, byte ignitionPin);
        gbc_settings getSettings();
        void setSettings(gbc_settings new_settings);
        void start();
        void stop();
        void update();
        GasBurnerControlState getState();


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
#endif