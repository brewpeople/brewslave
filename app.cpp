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
Ui ui{display, VERSION_STRING TEMPERATURE_MESSAGE CONTROLLER_MESSAGE};
#else
MockUi ui{};
#endif  // WITH_SH1106

Comm comm{controller};

class App {
public:
    App(Updateable& ui, Controller& controller)
    : m_ui{ui}
    , m_controller{controller}
    {}

    void update()
    {
        m_controller.update();

        const auto current_temperature{m_controller.temperature()};
        const auto target_temperature{m_controller.target_temperature()};
        const auto delta{current_temperature - m_last_temperature};
        uint8_t ui_state{0};

        if (delta > 0.1f) {
            ui_state |= Updateable::State::UpArrow;
        }

        if (delta < -0.1f) {
            ui_state |= Updateable::State::DownArrow;
        }

        if (m_controller.has_problem()) {
            ui_state |= Updateable::State::Warning;
        }

        m_ui.set_state(ui_state);

        m_last_temperature = current_temperature;
        m_last_target_temperature = target_temperature;

        switch (m_state) {
            case State::Main:
                m_ui.set_big_number(static_cast<uint8_t>(round(current_temperature)));
                m_ui.set_small_number(static_cast<uint8_t>(round(target_temperature)));
                m_ui.update();
                break;

            case State::SetTarget:
                break;
        }
    }

private:
    enum class State {
        Main,
        SetTarget,
    };

    Updateable& m_ui;
    Controller& m_controller;
    State m_state{State::Main};
    float m_last_temperature{20.0f};
    float m_last_target_temperature{20.0f};
};

App app{ui, controller};

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
    app.update();
}
