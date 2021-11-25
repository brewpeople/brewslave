#include "ui.h"
#include "fonts.h"

Ui::Ui(Sh1106& display, const char* welcome)
: m_display{display}
, m_pico{display}
, m_welcome{welcome}
, m_welcome_last{welcome}
{}

void Ui::set_big_number(uint8_t number)
{
    auto clamped = number >= 100 ? 99 : number;
    m_refresh = m_refresh || (m_big_number != clamped);
    m_big_number = clamped;
}

void Ui::set_small_number(uint8_t number)
{
    auto clamped = number >= 100 ? 99 : number;
    m_refresh = m_refresh || (m_small_number != clamped);
    m_small_number = clamped;
}

void Ui::set_state(uint8_t state)
{
    m_refresh = m_refresh || (m_state != state);
    m_state = state;
}

void Ui::update()
{
    // Bail out early if there is nothing to redraw.
    if (!m_refresh && (m_welcome == nullptr)) {
        return;
    }

    m_display.clear();

    if ((m_state & Updateable::State::UpArrow) != 0) {
        m_display.draw_bitmap(70, 0, Bitmap { 11, 6, ICON_ARROW_UP_11_6 });
    }

    if ((m_state & Updateable::State::DownArrow) != 0) {
        m_display.draw_bitmap(70, m_display.height - 1 - 6, Bitmap { 11, 6, ICON_ARROW_DOWN_11_6 });
    }

    if ((m_state & Updateable::State::Warning) != 0) {
        m_display.draw_bitmap(m_display.width - 1 - 22, m_display.height - 1 - 22, Bitmap { 22, 22, ICON_WARNING_22_22 });
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

    m_display.draw_bitmap(0, 0, Bitmap { 36, 64, DIGITS_36_64[m_big_number / 10] });
    m_display.draw_bitmap(36, 0, Bitmap { 36, 64, DIGITS_36_64[m_big_number % 10] });

    m_display.draw_bitmap(m_display.width - 1 - 2 * 18, 0, Bitmap { 18, 32, DIGITS_18_32[m_small_number / 10] });
    m_display.draw_bitmap(m_display.width - 1 - 1 * 18, 0, Bitmap { 18, 32, DIGITS_18_32[m_small_number % 10] });

    m_display.flush();
    m_refresh = false;
}
