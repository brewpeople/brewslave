#include "comm.h"
#include "controller.h"

namespace {
    enum class Command : uint8_t {
        invalid = 0x0,
        read_temperature = 0x1,
        write_temperature = 0x2,
        read_stirrer = 0x4,
        turn_stirrer_on = 0x5,
        turn_stirrer_off = 0x6,
        read_heater = 0x8,
    };

    enum class Response : uint8_t {
        ack = 0x80,
        nack = 0x40,
    };
}

uint8_t response(Command command, Response response)
{
    return static_cast<uint8_t>(command) | static_cast<uint8_t>(response);
}

Comm::Comm(Controller& controller)
    : m_controller{controller}
{}

void Comm::process_serial_data()
{
    Command command{Command::invalid};

    if (Serial.readBytes((char*) &command, 1) != 1) {
        return;
    }

    switch (command) {
        case Command::read_temperature:
            {
                float temperature{m_controller.temperature()};
                Serial.write((const uint8_t*) &temperature, 4);
            }
            break;
        case Command::write_temperature:
            {
                float temperature{20.0f};

                if (Serial.readBytes((char *) &temperature, 4) == 4) {
                    m_controller.set_temperature(temperature);
                    Serial.write(response(Command::write_temperature, Response::ack));
                }
                else {
                    Serial.write(response(Command::write_temperature, Response::nack));
                }
            }
            break;
        case Command::read_stirrer:
            Serial.write(static_cast<uint8_t>(m_controller.stirrer_is_on()));
            break;
        case Command::turn_stirrer_on:
            m_controller.set_stirrer_on(true);
            Serial.write(response(Command::turn_stirrer_on, Response::ack));
            break;
        case Command::turn_stirrer_off:
            m_controller.set_stirrer_on(false);
            Serial.write(response(Command::turn_stirrer_off, Response::ack));
            break;
        case Command::read_heater:
            Serial.write(static_cast<uint8_t>(m_controller.heater_is_on()));
            break;
        case Command::invalid:
            break;
    }

    Serial.flush();
}
