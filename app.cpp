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

        auto target_temperature{m_controller.target_temperature()};

        if (m_encoder.pressed()) {
            switch (m_state) {
                case State::SetTarget:
                    m_state = State::Main;
                    target_temperature = static_cast<float>(m_set_target_temperature);
                    m_controller.set_temperature(target_temperature);
                    break;
                case State::Main:
                    m_set_target_temperature = static_cast<uint8_t>(round(target_temperature));
                    m_state = State::SetTarget;
                    break;
            }
        }

        const auto current_temperature{m_controller.temperature()};
        const auto delta{current_temperature - m_last_temperature};
        m_last_temperature = current_temperature;

        switch (m_state) {
            case State::Main:
                m_ui_state &= ~(Updateable::State::SmallUpArrow | Updateable::State::SmallDownArrow | Updateable::State::SmallEq);
                m_ui.set_small_number(static_cast<uint8_t>(round(target_temperature)));
                break;

            case State::SetTarget:
                {
                    const auto direction{m_encoder.direction()};

                    if (direction == ButtonEncoder::Direction::Clockwise && m_set_target_temperature < 100) {
                        m_set_target_temperature++;
                    }
                    else if (direction == ButtonEncoder::Direction::CounterClockwise && m_set_target_temperature > 1) {
                        m_set_target_temperature--;
                    }

                    const auto current_target{static_cast<uint8_t>(round(target_temperature))};

                    if (m_set_target_temperature > current_target) {
                        m_ui_state &= ~(Updateable::State::SmallDownArrow | Updateable::State::SmallEq);
                        m_ui_state |= Updateable::State::SmallUpArrow;
                    }
                    else if (m_set_target_temperature < current_target) {
                        m_ui_state &= ~(Updateable::State::SmallUpArrow | Updateable::State::SmallEq);
                        m_ui_state |= Updateable::State::SmallDownArrow;
                    }
                    else {
                        m_ui_state &= ~(Updateable::State::SmallUpArrow | Updateable::State::SmallDownArrow);
                        m_ui_state |= Updateable::State::SmallEq;
                    }

                    m_ui.set_small_number(m_set_target_temperature);
                }
                break;
        }

        if (delta > 0.1f) {
            m_ui_state &= ~Updateable::State::DownArrow;
            m_ui_state |= Updateable::State::UpArrow;
        }

        if (delta < -0.1f) {
            m_ui_state &= ~Updateable::State::UpArrow;
            m_ui_state |= Updateable::State::DownArrow;
        }

        if (m_controller.has_problem()) {
            m_ui_state |= Updateable::State::Warning;
        }
        else {
            m_ui_state &= ~Updateable::State::Warning;
        }

        m_ui.set_big_number(static_cast<uint8_t>(round(current_temperature)));
        m_ui.set_state(m_ui_state);
        m_ui.update();
    }

private:
    enum class State {
        Main,
        SetTarget,
    };

    Updateable& m_ui;
    uint8_t m_ui_state{0};
    Controller& m_controller;
    ButtonEncoder& m_encoder;
    State m_state{State::Main};
    float m_last_temperature{20.0f};
    uint8_t m_set_target_temperature{0};
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
