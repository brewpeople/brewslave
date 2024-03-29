#include "comm.h"
#include "controller.h"

namespace {
    enum class Command : uint8_t {
        invalid = 0x0,
        read_state = 0x1,
        set_temperature = 0x2,
        turn_stirrer_on = 0x3,
        turn_stirrer_off = 0x4,
    };

    enum class Response : uint8_t {
        ack = 0x80,
        nack = 0x40,
    };

    uint8_t response(Command command, Response response) { return static_cast<uint8_t>(command) | static_cast<uint8_t>(response); }
}

Comm::Comm(Controller& controller)
: m_controller{controller}
{
}

void Comm::process_serial_data()
{
    Command command{Command::invalid};

    if (Serial.readBytes((char*) &command, 1) != 1) {
        return;
    }

    switch (command) {
        case Command::read_state: {
            const float current{m_controller.temperature()};
            const float target{m_controller.target_temperature()};
            uint8_t state{0};

            if (m_controller.stirrer_is_on()) {
                state |= 0x1;
            }

            if (m_controller.heater_is_on()) {
                state |= 0x2;
            }

            Serial.write((const uint8_t*) &current, 4);
            Serial.write((const uint8_t*) &target, 4);
            Serial.write(state);
        } break;
        case Command::set_temperature: {
            float temperature{20.0f};

            if (Serial.readBytes((char*) &temperature, 4) == 4) {
                m_controller.set_temperature(temperature);
                Serial.write(response(Command::set_temperature, Response::ack));
            }
            else {
                Serial.write(response(Command::set_temperature, Response::nack));
            }
        } break;
        case Command::turn_stirrer_on:
            m_controller.set_stirrer_on(true);
            Serial.write(response(Command::turn_stirrer_on, Response::ack));
            break;
        case Command::turn_stirrer_off:
            m_controller.set_stirrer_on(false);
            Serial.write(response(Command::turn_stirrer_off, Response::ack));
            break;
        case Command::invalid:
            break;
    }

    Serial.flush();
}
