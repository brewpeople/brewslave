#include "config.h"

#include <Arduino.h>

#include "burner.h"
#include "button.h"
#include "comm.h"
#include "controller.h"
#include "hotplate.h"
#include "sensor.h"
#include "ui.h"

#if defined(WITH_DS18B20)
#include <ds18b20.h>
#if defined(BREW_SENSOR_PIN)
#if defined(BREW_SENSOR_PIN_PULLUP)
Ds18b20 brew_sensor{BREW_SENSOR_PIN, BREW_SENSOR_PIN_PULLUP};
#else
Ds18b20 brew_sensor{BREW_SENSOR_PIN};
#endif // BREW_SENSOR_PIN_PULLUP
#else
MockTemperatureSensor brew_sensor;
#endif // BREW_SENSOR_PIN
#if defined(SPARGING_SENSOR_PIN)
Ds18b20 sparging_sensor{SPARGING_SENSOR_PIN};
#else
MockTemperatureSensor sparging_sensor;
#endif // SPARGING_SENSOR_PIN
#define TEMPERATURE_MESSAGE " +ds18b20"
#else // WITH_DS18B20
MockTemperatureSensor brew_sensor;
MockTemperatureSensor sparging_sensor;
#define TEMPERATURE_MESSAGE " +mock_sensor"
#endif // WITH_DS18B20

#if defined(WITH_KY040)
#include "ky040.h"
Ky040 encoder{KY040_SW, KY040_DT, KY040_CLK};
#define ENCODER_MESSAGE " +ky040"
#else
#include "encoder.h"
MockEncoder encoder{};
#define ENCODER_MESSAGE " +mock_encoder"
#endif

#if defined(WITH_BUTTONS)
#include "PushButton.h"
#if defined(BREW_BUTTON_PIN)
PushButton brew_button{BREW_BUTTON_PIN};
#else
MockButton brew_button{};
#endif // BREW_BUTTON_PIN
#if defined(SPARGING_BUTTON_PIN)
PushButton sparging_button{SPARGING_BUTTON_PIN};
#else
MockButton sparging_button{};
#endif // SPARGING_BUTTON_PIN
#else
MockButton brew_button{};
MockButton sparging_button{};
#endif // WITH_BUTTONS

#if defined(WITH_GBC)
#include <GasBurnerControl.h>

GasBurnerControl gbc{GBC_POWER_PIN, GBC_DEJAM_PIN, GBC_JAMMED_PIN, GBC_VALVE_PIN, GBC_IGNITION_PIN};
#define GBC_MESSAGE " +real_gbc"
#else
MockGasBurner gbc{};
#define GBC_MESSAGE " +mock_gbc"
#endif

#if defined(HOTPLATE_PIN)
#include <HotplateController.h>
HotplateController hotplate(HOTPLATE_PIN);
#else
MockHotplate hotplate{};
#endif

#if defined(WITH_MOCK_CONTROLLER)
MockController controller{};
#define CONTROLLER_MESSAGE " +mock_controller"
#else
MainController controller{brew_sensor, sparging_sensor, gbc, hotplate};
#define CONTROLLER_MESSAGE " +real_controller"
#endif

#if defined(WITH_SH1106)
#include "sh1106.h"

Sh1106 display{SH1106_RST, SH1106_DC, SH1106_DIN, SH1106_CLK};
#elif defined(WITH_SH1107)
#include "sh1107.h"

Sh1107 display{SH1107_RST, SH1107_DC, SH1107_DIN, SH1107_CLK};
#elif defined(WITH_SSD1327)
#include "ssd1327.h"

Ssd1327 display{SSD1327_RST, SSD1327_DC, SSD1327_DIN, SSD1327_CLK};
#else
MockDisplay display{};
#endif // WITH_SH1106

Ui ui{display, VERSION_STRING TEMPERATURE_MESSAGE ENCODER_MESSAGE CONTROLLER_MESSAGE GBC_MESSAGE};

ISR(PCINT1_vect)
{
    encoder.update();
}

void brew_button_trigger()
{
    brew_button.trigger();
}

void sparging_button_trigger()
{
    sparging_button.trigger();
}

Comm comm{controller};

class App {
public:
    App(Ui& ui, Controller& controller, TemperatureSensor& sparging_sensor, ButtonEncoder& encoder)
    : m_ui{ui}
    , m_controller{controller}
    , m_sparging_sensor{sparging_sensor}
    , m_encoder{encoder}
    , m_last_update{millis()}
    {
    }

    void update()
    {
        const auto now{millis()};
        const auto elapsed{now - m_last_update};
        m_last_update = now;

        m_controller.update(elapsed);

        auto brew_target_temperature{m_controller.brew_target_temperature()};

        if (m_encoder.pressed()) {
            switch (m_state) {
                case State::SetTarget:
                    m_state = State::Main;
                    brew_target_temperature = static_cast<float>(m_set_brew_target_temperature);
                    m_controller.set_brew_temperature(brew_target_temperature);
                    break;
                case State::Main:
                    m_set_brew_target_temperature = static_cast<uint8_t>(round(brew_target_temperature));
                    m_state = State::SetTarget;
                    break;
            }
        }

        const auto current_temperature{m_controller.brew_temperature()};
        const auto delta{current_temperature - m_last_temperature};
        m_last_temperature = current_temperature;

        const auto sparging_temperature{m_sparging_sensor.temperature()};

        switch (m_state) {
            case State::Main:
                m_ui_state &= ~(Ui::State::SmallUpArrow | Ui::State::SmallDownArrow | Ui::State::SmallEq);
                m_ui.set_small_number(static_cast<uint8_t>(round(brew_target_temperature)));
                break;

            case State::SetTarget: {
                const auto direction{m_encoder.direction()};

                if (direction == ButtonEncoder::Direction::Clockwise && m_set_brew_target_temperature < 100) {
                    m_set_brew_target_temperature++;
                }
                else if (direction == ButtonEncoder::Direction::CounterClockwise && m_set_brew_target_temperature > 0) {
                    m_set_brew_target_temperature--;
                }

                const auto current_target{static_cast<uint8_t>(round(brew_target_temperature))};

                if (m_set_brew_target_temperature > current_target) {
                    m_ui_state &= ~(Ui::State::SmallDownArrow | Ui::State::SmallEq);
                    m_ui_state |= Ui::State::SmallUpArrow;
                }
                else if (m_set_brew_target_temperature < current_target) {
                    m_ui_state &= ~(Ui::State::SmallUpArrow | Ui::State::SmallEq);
                    m_ui_state |= Ui::State::SmallDownArrow;
                }
                else {
                    m_ui_state &= ~(Ui::State::SmallUpArrow | Ui::State::SmallDownArrow);
                    m_ui_state |= Ui::State::SmallEq;
                }

                m_ui.set_small_number(m_set_brew_target_temperature);
            } break;
        }

        if (delta > 0.1f) {
            m_ui_state &= ~Ui::State::DownArrow;
            m_ui_state |= Ui::State::UpArrow;
        }

        if (delta < -0.1f) {
            m_ui_state &= ~Ui::State::UpArrow;
            m_ui_state |= Ui::State::DownArrow;
        }

        if (m_controller.has_problem()) {
            m_ui_state |= Ui::State::Warning;
        }
        else {
            m_ui_state &= ~Ui::State::Warning;
        }

        if (brew_sensor.is_connected()) {
            m_ui.set_big_number(static_cast<uint8_t>(round(current_temperature)));
        }
        else {
            m_ui.set_big_number(0);
            m_ui_state &= ~Ui::State::DownArrow;
            m_ui_state &= ~Ui::State::UpArrow;
        }

        brew_button.update();
        if (brew_button.pressed()) {
            controller.set_brew_temperature(0.0f); // deactivates controller
            (gbc.state() == GasBurner::State::idle) ? gbc.start() : gbc.stop();
        }

        sparging_button.update();
        if (sparging_button.pressed()) {
            controller.set_sparging_temperature(0.0f); // deactivate controller
            hotplate.state() ? hotplate.stop() : hotplate.start();
        }

        m_ui.set_state(m_ui_state);
        m_ui.update();
    }

private:
    enum class State {
        Main,
        SetTarget,
    };

    Ui& m_ui;
    uint8_t m_ui_state{0};
    Controller& m_controller;
    TemperatureSensor& m_sparging_sensor;
    ButtonEncoder& m_encoder;
    State m_state{State::Main};
    float m_last_temperature{20.0f};
    uint8_t m_set_brew_target_temperature{0};
    unsigned long m_last_update{0};
};

App app{ui, controller, sparging_sensor, encoder};

void setup()
{
    // Disable L LED.
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);

    Serial.begin(115200, SERIAL_8N1);

#if defined(BREW_BUTTON_PIN)
    attachInterrupt(digitalPinToInterrupt(BREW_BUTTON_PIN), brew_button_trigger, RISING);
#endif
#if defined(SPARGING_BUTTON_PIN)
    attachInterrupt(digitalPinToInterrupt(SPARGING_BUTTON_PIN), sparging_button_trigger, RISING);
#endif

    brew_sensor.begin();
    sparging_sensor.begin();

    display.begin();
    gbc.begin();
    hotplate.begin(); // ensure that relay is off at start
}

void serialEvent()
{
    comm.process_serial_data();
}

void loop()
{
    app.update();
}
