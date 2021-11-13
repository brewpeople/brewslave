#include "config.h"

#include <Arduino.h>

#include "comm.h"
#include "controller.h"
#include "sensor.h"

#if WITH_DS18B20
Ds18b20 sensor{DS18B20_PIN};
#else
MockTemperatureSensor sensor;
#endif  // WITH_DS18B20

MockController controller{sensor};

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
