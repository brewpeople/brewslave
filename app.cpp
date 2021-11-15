#include "config.h"

#include <Arduino.h>

#include "comm.h"
#include "controller.h"
#include "sensor.h"

#if WITH_SH1106
#include "sh1106.h"
#include "fonts.h"

Sh1106 display{SH1106_RST, SH1106_DC, SH1106_CS, SH1106_DIN, SH1106_CLK};
#endif  // WITH_SH1106

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

#if WITH_SH1106
    display.begin();
#endif  // WITH_SH1106
}

void serialEvent()
{
    comm.process_serial_data();
}

void loop()
{
#if defined(WITH_SH1106)
    // This is just a demonstration, we should definitely not update the display
    // in the tight loop because it slows down everything (e.g. it takes 2.5s
    // for the serial line to respond).
    display.clear();

    auto temperature{static_cast<uint8_t>(sensor.temperature())};
    temperature = temperature >= 100 ? 99 : temperature;

    display.draw_bitmap(0, 0, Bitmap { 36, 64, HUGE_DIGITS[temperature / 10] });
    display.draw_bitmap(36, 0, Bitmap { 36, 64, HUGE_DIGITS[temperature % 10] });

    display.flush();
#endif  // WITH_SH1106
}
