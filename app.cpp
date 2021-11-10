#include <Arduino.h>
#include "config.h"
#include "comm.h"
#include "controller.h"

#if WITH_MOCK_CONTROLLER
MockController controller;
#else
NonExistingDisplay display;
// This should not only take the display but also the state machine, input
// devices etc.
NonExistingController controller{display};
#endif

Comm comm{controller};

void setup()
{
    // Disable L LED.
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);

    Serial.begin(115200, SERIAL_8N1);
}

void serialEvent()
{
    comm.process_serial_data();
}

void loop()
{
}
