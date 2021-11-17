#include "ui.h"
#include "fonts.h"

Ui::Ui(Sh1106& display, Controller& controller)
: m_display{display}
, m_controller{controller}
{}

void Ui::update()
{
    const auto current_temperature{m_controller.temperature()};
    const auto target_temperature{m_controller.target_temperature()};
    const auto delta{current_temperature - m_last_temperature};
    const bool refresh{
        // Refresh if current temperature has changed
        fabs(delta) > 0.0f ||
        // Refresh if target temperature has changed
        (target_temperature != m_last_target_temperature)
    };

    if (refresh) {
        m_display.clear();
    }

    if (delta > 0.1f) {
        m_display.draw_bitmap(70, 0, Bitmap { 11, 6, ICON_ARROW_UP_11_6 });
    }

    if (delta < - 0.1f) {
        m_display.draw_bitmap(70, 63 - 6, Bitmap { 11, 6, ICON_ARROW_DOWN_11_6 });
    }

    if (refresh) {
        auto clamped{static_cast<uint8_t>(round(current_temperature))};
        clamped = clamped >= 100 ? 99 : clamped;

        m_display.draw_bitmap(0, 0, Bitmap { 36, 64, DIGITS_36_64[clamped / 10] });
        m_display.draw_bitmap(36, 0, Bitmap { 36, 64, DIGITS_36_64[clamped % 10] });

        clamped = static_cast<uint8_t>(target_temperature);
        clamped = clamped >= 100 ? 99 : clamped;
        m_display.draw_bitmap(86, 0, Bitmap { 18, 32, DIGITS_18_32[clamped / 10] });
        m_display.draw_bitmap(106, 0, Bitmap { 18, 32, DIGITS_18_32[clamped % 10] });

        m_display.flush();

        m_last_target_temperature = target_temperature;
        m_last_temperature = current_temperature;
    }
}