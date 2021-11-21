#include "ui.h"
#include "fonts.h"

Ui::Ui(Sh1106& display, Controller& controller, const char* welcome)
: m_display{display}
, m_pico{display}
, m_controller{controller}
, m_welcome{welcome}
, m_welcome_last{welcome}
{}

void Ui::update()
{
    const auto current_temperature{m_controller.temperature()};
    const auto target_temperature{m_controller.target_temperature()};
    const auto delta{current_temperature - m_last_temperature};
    const auto have_problem{m_controller.has_problem()};
    const bool refresh{
        // Refresh if current temperature has changed
        fabs(delta) > 0.0f ||
        // Refresh if target temperature has changed
        (target_temperature != m_last_target_temperature) ||
        have_problem ||
        m_welcome_last != nullptr
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

    if (have_problem) {
        m_display.draw_bitmap(127 - 22, 63 - 22, Bitmap { 22, 22, ICON_WARNING_22_22 });
    }

    if (*m_welcome_last != '\0') {
        // This is pretty choppy because of the uneven loop timing. There are
        // two options: we schedule the UI updates at precise points in time or
        // use ye olde trick of time-dependent updates. But not super important
        // for now, I'd say.
        m_pico.draw(m_welcome_last, m_current_scroll_start, 63 - 6);

        if (m_current_scroll_start > 69) {
            m_current_scroll_start--;
        }
        else {
            m_welcome_last++;
            m_current_scroll_start += 4;
        }
    }

    if (refresh) {
        auto clamped{static_cast<uint8_t>(round(current_temperature))};
        clamped = clamped >= 100 ? 99 : clamped;

        m_display.draw_bitmap(0, 0, Bitmap { 36, 64, DIGITS_36_64[clamped / 10] });
        m_display.draw_bitmap(36, 0, Bitmap { 36, 64, DIGITS_36_64[clamped % 10] });

        clamped = static_cast<uint8_t>(target_temperature);
        clamped = clamped >= 100 ? 99 : clamped;
        m_display.draw_bitmap(127 - 2 * 18, 0, Bitmap { 18, 32, DIGITS_18_32[clamped / 10] });
        m_display.draw_bitmap(127 - 1 * 18, 0, Bitmap { 18, 32, DIGITS_18_32[clamped % 10] });

        m_display.flush();

        m_last_target_temperature = target_temperature;
        m_last_temperature = current_temperature;
    }
}
