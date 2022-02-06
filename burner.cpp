#include "burner.h"

uint16_t GasBurner::encode_state(State state, uint8_t dejam_counter, uint8_t ignition_counter)
{
    return static_cast<uint8_t>(state) | ((dejam_counter & 0x1F) << 11) | ((ignition_counter & 0x1F) << 6);
}

void MockGasBurner::begin() {}

void MockGasBurner::start() {}

void MockGasBurner::stop() {}

void MockGasBurner::update()
{
    const auto now{millis()};

    if (now - m_last_state_change_time >= 2000) {
        m_last_state_change_time = now;
        uint8_t mock_main_state = rand() % 6;
        uint8_t mock_substate = 0;

        if (mock_main_state == 4) {
            mock_substate = rand() % 4 + 1;
        }
        else if (mock_main_state == 5) {
            mock_substate = rand() % 3 + 1;
        }

        m_state = static_cast<GasBurner::State>(mock_main_state * 10 + mock_substate);

        if (mock_main_state > 0) {
            m_ignition_counter = rand() % 4;
            m_dejam_counter = rand() % 4;
        }
        else {
            m_ignition_counter = 0;
            m_dejam_counter = 0;
        }
    }
}

GasBurner::State MockGasBurner::state()
{
    return m_state;
}

unsigned int MockGasBurner::full_state()
{
    return GasBurner::encode_state(m_state, m_dejam_counter, m_ignition_counter);
}
