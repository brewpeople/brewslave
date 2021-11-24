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

#if defined(WITH_KY040)
#include "ky040.h"
Ky040 encoder{KY040_SW, KY040_DT, KY040_CLK};
#define ENCODER_MESSAGE " +ky040"
#else
#include "encoder.h"
MockEncoder encoder{};
#define ENCODER_MESSAGE " +mock_encoder"
#endif

#if defined(WITH_SH1106)
#include "sh1106.h"
#include "ui.h"

Sh1106 display{SH1106_RST, SH1106_DC, SH1106_CS, SH1106_DIN, SH1106_CLK};
Ui ui{display, VERSION_STRING TEMPERATURE_MESSAGE ENCODER_MESSAGE CONTROLLER_MESSAGE};
#else
MockUi ui{};
#endif  // WITH_SH1106

ISR(PCINT1_vect)
{
    encoder.update();
}

Comm comm{controller};

class App {
public:
    App(Updateable& ui, Controller& controller, ButtonEncoder& encoder)
    : m_ui{ui}
    , m_controller{controller}
    , m_encoder{encoder}
    {}

    void update()
    {
        m_controller.update();

        const auto current_temperature{m_controller.temperature()};
        const auto delta{current_temperature - m_last_temperature};
        uint8_t ui_state{0};
        auto target_temperature{m_controller.target_temperature()};

        switch (m_encoder.direction()) {
            case ButtonEncoder::Direction::Clockwise:
                target_temperature = min(100.0f, target_temperature + 1.0f);
                break;
            case ButtonEncoder::Direction::CounterClockwise:
                target_temperature = max(0.0f, target_temperature - 1.0f);
                break;
            default:
                break;
        }

        m_controller.set_temperature(target_temperature);
        m_last_temperature = current_temperature;

        if (delta > 0.1f) {
            ui_state |= Updateable::State::UpArrow;
        }

        if (delta < -0.1f) {
            ui_state |= Updateable::State::DownArrow;
        }

        if (m_controller.has_problem()) {
            ui_state |= Updateable::State::Warning;
        }

        m_ui.set_big_number(static_cast<uint8_t>(round(current_temperature)));
        m_ui.set_small_number(static_cast<uint8_t>(round(target_temperature)));
        m_ui.set_state(ui_state);
        m_ui.update();
    }

private:
    enum class State {
        Main,
        SetTarget,
    };

    Updateable& m_ui;
    Controller& m_controller;
    ButtonEncoder& m_encoder;
    float m_last_temperature{20.0f};
};

App app{ui, controller, encoder};

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
