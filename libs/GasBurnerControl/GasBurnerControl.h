#ifndef GasBurnerControl_h
#define GasBurnerControl_h


#include "Arduino.h"


/* OPTIONS */
#define GBC_N_DEJAM_ATTEMPTS 3      // number of unsuccessful dejam attempts before aborting permanently (1-255)
#define GBC_N_IGNITION_ATTEMPTS 3   // number of unsuccessful ignition attempts before aborting permanently (1-255)

#define GBC_HIGH 0                  // logical high (0: GND, 1: VCC)
#define GBC_LOW 1                   // logical low (0: GND, 1: VCC)

#define GBC_SERIAL_DEBUG            // if uncommented every update will also send a message over serial


/* TIMER SETTINGS */
#define GBC_START_DELAY 2           // s, initial delay after powering the GBC
#define GBC_DEJAM_DELAY 65          // s, wait time until dejamming is possible
#define GBC_DEJAM_DELAY2 10         // s, wait time between additional dejam attempts
#define GBC_IGNITION_DELAY 22       // s, time after which ignition should be complete
#define GBC_DEJAM_DURATION 1000     // ms, duration of button press during dejamming
#define GBC_POST_DEJAM_DELAY 1000   // ms, delay after dejam button release


/* STATE REPRESENTATIONS (must be unique) */
#define GBC_IDLE 0
#define GBC_STARTING 1
#define GBC_IGNITION 2
#define GBC_RUNNING 3
#define GBC_DEJAM 4
#define GBC_ERROR 5


class GasBurnerControl
{
    public:
        GasBurnerControl(byte powerPin, byte dejamPin, byte jammedPin, byte valvePin, byte ignitionPin);
        void start();
        void stop();
        void update();
        byte getState();


    private:
        byte _powerPin;
        byte _dejamPin;
        byte _jammedPin;
        byte _valvePin;
        byte _ignitionPin;
    
        byte _ignitionCounter;
        byte _dejamCounter;
    
        unsigned long _startTime;
        unsigned long _ignitionStartTime;
        unsigned long _nextDejamAttemptTime;
        unsigned long _dejamTimer;
        
        byte _state;
        
        bool _valve;
        bool _jammed;
        bool _ignition;

        void _dejam(unsigned int delay_s);
};
#endif