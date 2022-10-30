#include "ui.h"
#include "fonts.h"

Ui::Ui(Display& display, const char* welcome)
: m_display{display}
, m_pico{display}
, m_welcome{welcome}
, m_welcome_last{welcome}
{
}

void Ui::set_layout_switching(bool enable)
{
    m_layout_switching = enable;
    // If layout switching is disabled, default to layout A (true)
    // m_current_layout = m_layout_switching ? m_current_layout : LayoutA;
}

Ui::Layout Ui::freeze_layout(bool freeze)
{
    m_freeze_layout = freeze;
    if (!m_freeze_layout) {
        m_last_layout_switch = millis();
    }
    return m_current_layout;
}

Ui::Layout Ui::current_layout()
{
    return m_current_layout;
}

void Ui::set_layout(Layout layout)
{
    m_current_layout = layout;
}

void Ui::set_big_number_a(uint8_t number)
{
    auto clamped = number >= 100 ? 99 : number;
    m_refresh = m_refresh || (m_big_number_a != clamped);
    m_big_number_a = clamped;
}

void Ui::set_big_number_b(uint8_t number)
{
    auto clamped = number >= 100 ? 99 : number;
    m_refresh = m_refresh || (m_big_number_b != clamped);
    m_big_number_b = clamped;
}

void Ui::set_small_number_a(uint8_t number)
{
    auto clamped = number >= 100 ? 99 : number;
    m_refresh = m_refresh || (m_small_number_a != clamped);
    m_small_number_a = clamped;
}

void Ui::set_small_number_b(uint8_t number)
{
    auto clamped = number >= 100 ? 99 : number;
    m_refresh = m_refresh || (m_small_number_b != clamped);
    m_small_number_b = clamped;
}

void Ui::set_state(uint8_t state)
{
    m_refresh = m_refresh || (m_state != state);
    m_state = state;
}

void Ui::set_full_burner_state(uint16_t state)
{
    m_refresh = m_refresh || (m_full_burner_state != state);
    m_full_burner_state = state;
}

void Ui::update()
{
    const auto now{millis()};

    // Bail out early if there is nothing to redraw.
    if ((now - m_last_update) < 15 || (!m_refresh && (m_welcome == nullptr))) {
        return;
    }

    m_last_update = now;

    // Switch between different layouts
    if (m_layout_switching && !m_freeze_layout) {
        switch (m_current_layout) {
            case LayoutA:
                if (m_last_layout_switch + 5000 < now) {
                    m_current_layout = LayoutB;
                    m_last_layout_switch = now;
                }
                break;
            case LayoutB:
                if (m_last_layout_switch + 2000 < now) {
                    m_current_layout = LayoutA;
                    m_last_layout_switch = now;
                }
            default:
                break;
        }
    }

    do {
        m_display.clear();

        // switch between two layouts for brew and sparging
        switch (m_current_layout) {
            case LayoutA: {
                if ((m_state & State::UpArrowA) != 0) {
                    m_display.draw_bitmap(70, 0, Bitmap{11, 6, ICON_ARROW_UP_11_6});
                }

                if ((m_state & State::DownArrowA) != 0) {
                    m_display.draw_bitmap(70, m_display.height - 1 - 6, Bitmap{11, 6, ICON_ARROW_DOWN_11_6});
                }

                // if ((m_state_a & State::Warning) != 0) {
                //     m_display.draw_bitmap(m_display.width - 1 - 22, m_display.height - 1 - 22, Bitmap{22, 22, ICON_WARNING_22_22});
                // }

                GasBurner::decoded_state burner_decoded_state = GasBurner::decode_full_state(m_full_burner_state);

                // INFO: Expose more details on gbc burner state until we are confident it works and may want to return to simple/clean UI.
                switch (burner_decoded_state.state) {
                    case GasBurner::State::idle:
                        break;
                    case GasBurner::State::running:
                        m_display.draw_bitmap(m_display.width - 1 - 22, m_display.height - 1 - 22, Bitmap{22, 22, ICON_HEAT_22_22});
                        break;
                    case GasBurner::State::starting:
                        m_display.draw_bitmap(m_display.width - 1 - 22, m_display.height - 1 - 22, Bitmap{22, 22, ICON_COCK_22_22});
                        break;
                    case GasBurner::State::ignition:
                        m_display.draw_bitmap(90, 40, Bitmap{8, 10, ICON_PICO_BOLT_8_10});
                        m_pico.draw_char(char(burner_decoded_state.ignition_counter + '0'), 100, 43);
                        m_display.draw_bitmap(m_display.width - 1 - 22, m_display.height - 1 - 22, Bitmap{22, 22, ICON_COCK_22_22});
                        break;
                    case GasBurner::State::any_dejam:
                    case GasBurner::State::dejam_pre_delay:
                    case GasBurner::State::dejam_start:
                    case GasBurner::State::dejam_button_pressed:
                    case GasBurner::State::dejam_post_delay:
                        m_display.draw_bitmap(90, 40, Bitmap{8, 10, ICON_PICO_BOLT_8_10});
                        m_pico.draw_char(char(burner_decoded_state.ignition_counter + '0'), 100, 43);
                        m_display.draw_bitmap(90, m_display.height - 1 - 10, Bitmap{8, 10, ICON_PICO_LOCK_8_10});
                        m_pico.draw_char(char(burner_decoded_state.dejam_counter + '0'), 100, m_display.height - 1 - 7);
                        m_display.draw_bitmap(m_display.width - 1 - 22, m_display.height - 1 - 22, Bitmap{22, 22, ICON_COCK_22_22});
                        break;
                    case GasBurner::State::any_error:
                    case GasBurner::State::error_start:
                        m_display.draw_bitmap(90, 40, Bitmap{8, 10, ICON_PICO_BOLT_8_10});
                        m_pico.draw_char(char(burner_decoded_state.ignition_counter + '0'), 100, 43);
                        m_display.draw_bitmap(90, m_display.height - 1 - 10, Bitmap{8, 10, ICON_PICO_LOCK_8_10});
                        m_pico.draw_char(char(burner_decoded_state.dejam_counter + '0'), 100, m_display.height - 1 - 7);
                        m_display.draw_bitmap(m_display.width - 1 - 22, m_display.height - 1 - 22, Bitmap{22, 22, ICON_WARNING_22_22});
                        break;
                    case GasBurner::State::error_ignition:
                        m_display.draw_bitmap(90, 40, Bitmap{8, 10, ICON_PICO_BOLT_8_10});
                        m_pico.draw_char(char(burner_decoded_state.ignition_counter + '0'), 100, 43);
                        m_display.draw_bitmap(m_display.width - 1 - 22, m_display.height - 1 - 22, Bitmap{22, 22, ICON_WARNING_22_22});
                        break;
                    case GasBurner::State::error_dejam:
                        m_display.draw_bitmap(90, m_display.height - 1 - 10, Bitmap{8, 10, ICON_PICO_LOCK_8_10});
                        m_pico.draw_char(char(burner_decoded_state.dejam_counter + '0'), 100, m_display.height - 1 - 7);
                        m_display.draw_bitmap(m_display.width - 1 - 22, m_display.height - 1 - 22, Bitmap{22, 22, ICON_WARNING_22_22});
                        break;
                    case GasBurner::State::error_other:
                        m_display.draw_bitmap(m_display.width - 1 - 22, m_display.height - 1 - 22, Bitmap{22, 22, ICON_WARNING_22_22});
                        break;
                }

                if (m_big_number_a != 0) {
                    m_display.draw_bitmap(0, 0, Bitmap{36, 64, DIGITS_36_64[m_big_number_a / 10]});
                    m_display.draw_bitmap(36, 0, Bitmap{36, 64, DIGITS_36_64[m_big_number_a % 10]});
                }
                else {
                    m_display.draw_bitmap(0, 30, Bitmap{36, 4, DASH_36_4});
                    m_display.draw_bitmap(36, 30, Bitmap{36, 4, DASH_36_4});
                }

                if ((m_state & State::SmallUpArrow) != 0) {
                    m_display.draw_bitmap(m_display.width - 1 - 2 * 18 - 8, 0, Bitmap{6, 3, ICON_SMALL_ARROW_UP_6_3});
                }

                if ((m_state & State::SmallDownArrow) != 0) {
                    m_display.draw_bitmap(m_display.width - 1 - 2 * 18 - 8, 29, Bitmap{6, 3, ICON_SMALL_ARROW_DOWN_6_3});
                }

                if ((m_state & State::SmallEq) != 0) {
                    m_display.draw_bitmap(m_display.width - 1 - 2 * 18 - 8, 14, Bitmap{6, 5, ICON_SMALL_ARROW_EQ_6_5});
                }

                if (m_small_number_a != 0) {
                    m_display.draw_bitmap(m_display.width - 1 - 2 * 18, 0, Bitmap{18, 32, DIGITS_18_32[m_small_number_a / 10]});
                    m_display.draw_bitmap(m_display.width - 1 - 1 * 18, 0, Bitmap{18, 32, DIGITS_18_32[m_small_number_a % 10]});
                }
                else {
                    m_display.draw_bitmap(m_display.width - 1 - 2 * 18, 15, Bitmap{18, 2, DASH_18_2});
                    m_display.draw_bitmap(m_display.width - 1 - 1 * 18, 15, Bitmap{18, 2, DASH_18_2});
                }
                break;
            }

            case LayoutB: {
                if ((m_state & State::UpArrowB) != 0) {
                    m_display.draw_bitmap(46, 0, Bitmap{11, 6, ICON_ARROW_UP_11_6});
                }

                if ((m_state & State::DownArrowB) != 0) {
                    m_display.draw_bitmap(46, m_display.height - 1 - 6, Bitmap{11, 6, ICON_ARROW_DOWN_11_6});
                }

                if (m_big_number_b != 0) {
                    m_display.draw_bitmap(m_display.width - 1 - 32 - 36, 0, Bitmap{36, 64, DIGITS_36_64[m_big_number_b / 10]});
                    m_display.draw_bitmap(m_display.width - 1 - 32, 0, Bitmap{36, 64, DIGITS_36_64[m_big_number_b % 10]});
                }
                else {
                    m_display.draw_bitmap(m_display.width - 1 - 32 - 36, 30, Bitmap{36, 4, DASH_36_4});
                    m_display.draw_bitmap(m_display.width - 1 - 32, 30, Bitmap{36, 4, DASH_36_4});
                }

                if ((m_state & State::SmallUpArrow) != 0) {
                    m_display.draw_bitmap(38, 0, Bitmap{6, 3, ICON_SMALL_ARROW_UP_6_3});
                }

                if ((m_state & State::SmallDownArrow) != 0) {
                    m_display.draw_bitmap(38, 29, Bitmap{6, 3, ICON_SMALL_ARROW_DOWN_6_3});
                }

                if ((m_state & State::SmallEq) != 0) {
                    m_display.draw_bitmap(38, 14, Bitmap{6, 5, ICON_SMALL_ARROW_EQ_6_5});
                }

                if (m_small_number_b != 0) {
                    m_display.draw_bitmap(0, 0, Bitmap{18, 32, DIGITS_18_32[m_small_number_b / 10]});
                    m_display.draw_bitmap(18, 0, Bitmap{18, 32, DIGITS_18_32[m_small_number_b % 10]});
                }
                else {
                    m_display.draw_bitmap(0, 15, Bitmap{18, 2, DASH_18_2});
                    m_display.draw_bitmap(18, 15, Bitmap{18, 2, DASH_18_2});
                }
                break;
            }

            default:
                break;
        }

        if (*m_welcome_last != '\0') {
            // This is pretty choppy because of the uneven loop timing. There are
            // two options: we schedule the UI updates at precise points in time or
            // use ye olde trick of time-dependent updates. But not super important
            // for now, I'd say.
            m_pico.draw(m_welcome_last, m_current_scroll_start, 63 - 6);
        }

        m_display.flush();
    } while (m_display.next_segment());

    if (*m_welcome_last != '\0') {
        if (m_current_scroll_start > 69) {
            m_current_scroll_start--;
        }
        else {
            m_welcome_last++;
            m_current_scroll_start += 4;
        }
    }

    m_refresh = false;
}
