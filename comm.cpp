#include "comm.h"
#include "controller.h"

namespace {
    enum class Command : uint8_t {
        invalid = 0x0,
        read_state = 0x1,
        set_brew_temperature = 0x2,
        set_sparging_temperature = 0x3,
        read_burner_full_state = 0x4,
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
            const float brew_current{m_controller.brew_temperature()};
            const float brew_target{m_controller.brew_target_temperature()};
            const float sparging_current{m_controller.sparging_temperature()};
            const float sparging_target{m_controller.sparging_target_temperature()};
            uint8_t state{(uint8_t) m_controller.burner_state()}; // simple burner state occupies lower 6 bits

            if (m_controller.sparging_heater_is_on()) {
                state |= 0x1 << 7; // simple burner state occupies lower 6 bits
            }

            if (m_controller.brew_is_connected()) {
                Serial.write((const uint8_t*) &brew_current, 4);
            }
            else {
                // 0x7fffffff corresponds to IEEE 754 NaN
                float nan;
                unsigned long* p_nan = (unsigned long*) &nan;
                *p_nan = 0x7fffffff;
                Serial.write((const uint8_t*) &nan, 4);
            }
            Serial.write((const uint8_t*) &brew_target, 4);

            if (m_controller.sparging_is_connected()) {
                Serial.write((const uint8_t*) &sparging_current, 4);
            }
            else {
                // 0x7fffffff corresponds to IEEE 754 NaN
                float nan;
                unsigned long* p_nan = (unsigned long*) &nan;
                *p_nan = 0x7fffffff;
                Serial.write((const uint8_t*) &nan, 4);
            }
            Serial.write((const uint8_t*) &sparging_target, 4);

            Serial.write(state);
        } break;
        case Command::set_brew_temperature: {
            float temperature{20.0f};

            if (Serial.readBytes((char*) &temperature, 4) == 4) {
                m_controller.set_brew_temperature(temperature);
                Serial.write(response(Command::set_brew_temperature, Response::ack));
            }
            else {
                Serial.write(response(Command::set_brew_temperature, Response::nack));
            }
        } break;
        case Command::set_sparging_temperature: {
            float temperature{20.0f};

            if (Serial.readBytes((char*) &temperature, 4) == 4) {
                m_controller.set_sparging_temperature(temperature);
                Serial.write(response(Command::set_sparging_temperature, Response::ack));
            }
            else {
                Serial.write(response(Command::set_sparging_temperature, Response::nack));
            }
        } break;
        case Command::read_burner_full_state: {
            const auto full_state{m_controller.full_burner_state()};
            Serial.write((const uint8_t*) &full_state, 2);
        } break;
        case Command::invalid:
            break;
    }

    Serial.flush();
}
