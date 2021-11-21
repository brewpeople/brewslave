#include "config.h"

#include <Arduino.h>

#include "comm.h"
#include "controller.h"
#include "sensor.h"

#if defined(WITH_DS18B20)
#include <ds18b20.h>
Ds18b20 sensor{DS18B20_PIN};
#define TEMPERATURE_MESSAGE " +ds18b20"
#else
MockTemperatureSensor sensor;
#define TEMPERATURE_MESSAGE " +mock_sensor"
#endif  // WITH_DS18B20

#if defined(WITH_MOCK_CONTROLLER)
MockController controller{};
#define CONTROLLER_MESSAGE " +mock_controller"
#else
MainController controller{sensor};
#define CONTROLLER_MESSAGE " +real_controller"
#endif

#if defined(WITH_SH1106)
#include "sh1106.h"
#include "ui.h"

Sh1106 display{SH1106_RST, SH1106_DC, SH1106_CS, SH1106_DIN, SH1106_CLK};
Ui ui{display, controller, VERSION_STRING TEMPERATURE_MESSAGE CONTROLLER_MESSAGE};
#endif  // WITH_SH1106

Comm comm{controller};

void setup()
{
    // Disable L LED.
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);

    Serial.begin(115200, SERIAL_8N1);

#if defined(WITH_SH1106)
    display.begin();
#endif  // WITH_SH1106
}

void serialEvent()
{
    comm.process_serial_data();
}

void loop()
{
    controller.update();

#if defined(WITH_SH1106)
    ui.update();
#endif  // WITH_SH1106
}
