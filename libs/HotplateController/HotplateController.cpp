#include "HotplateController.h"

namespace hotplate {
    constexpr int high{0};
    constexpr int low{1};
}

HotplateController::HotplateController(uint8_t pin)
: m_pin{pin}
{
}

void HotplateController::begin()
{
    // Ensure that hotplate is turned off at start.
    digitalWrite(m_pin, hotplate::low);
    pinMode(m_pin, OUTPUT);
}

void HotplateController::start()
{
    digitalWrite(m_pin, hotplate::high);
}

void HotplateController::stop()
{
    digitalWrite(m_pin, hotplate::low);
}

bool HotplateController::state()
{
    return digitalRead(m_pin) ? false : true;
}