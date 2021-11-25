#include "fonts.h"

FontPico::FontPico(Sh1106& display)
: m_display{display}
{}

void FontPico::draw_char(char c, uint8_t x_start, uint8_t y_start)
{
    size_t offset{0};

    if (c >= '0' && c <= '9') {
        offset = c - '0';
    }
    else if (c >= 'a' && c <= 'z') {
        offset = c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'Z') {
        offset = c - 'A' + 10;
    }
    else if (c == '!') {
        offset = 36;
    }
    else if (c == '?') {
        offset = 37;
    }
    else if (c == '+') {
        offset = 38;
    }
    else if (c == ',') {
        offset = 39;
    }
    else if (c == '-') {
        offset = 40;
    }
    else if (c == '.') {
        offset = 41;
    }
    else if (c == '/') {
        offset = 42;
    }
    else if (c == ':') {
        offset = 43;
    }
    else if (c == ';') {
        offset = 44;
    }
    else if (c == '_') {
        offset = 45;
    }
    else if (c == '(') {
        offset = 46;
    }
    else if (c == ')') {
        offset = 47;
    }
    else {
        // skip unknown characters
        return;
    }

    const uint8_t x_end{static_cast<uint8_t>(min(x_start + 4, 127))};
    const uint8_t y_end{static_cast<uint8_t>(min(y_start + 6, 63))};

    for (uint8_t y = y_start; y < y_end; y++) {
        const uint8_t data{pgm_read_byte(PICO_FONT_4_6 + offset)};

        for (uint8_t x = x_start, bit = 0x80; x < x_end; x++, bit >>= 1) {
            if (data & bit) {
                m_display.draw_pixel(x, y);
            }
        }

        offset += 48;
    }
}

void FontPico::draw(const char* s, uint8_t x_start, uint8_t y)
{
    uint8_t x{x_start};

    for (size_t i = 0; s[i] != '\0'; i++) {
        if (s[i] != ' ') {
            draw_char(s[i], x, y);
        }

        x += 4;
    }
}

const PROGMEM uint8_t DIGITS_36_64[10][320] = {
{
  0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x7f, 0xfe,
  0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x01,
  0xff, 0xff, 0xc0, 0x00, 0x03, 0xff, 0xff, 0xc0,
  0x00, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x0f, 0xf8,
  0x1f, 0xf0, 0x00, 0x1f, 0xe0, 0x07, 0xf8, 0x00,
  0x1f, 0xc0, 0x03, 0xf8, 0x00, 0x3f, 0x80, 0x01,
  0xfc, 0x00, 0x3f, 0x80, 0x01, 0xfc, 0x00, 0x3f,
  0x00, 0x00, 0xfc, 0x00, 0x7f, 0x00, 0x00, 0xfe,
  0x00, 0x7f, 0x00, 0x00, 0xfe, 0x00, 0x7f, 0x00,
  0x00, 0xfe, 0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00,
  0xfe, 0x00, 0x00, 0x7e, 0x00, 0xfe, 0x00, 0x00,
  0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe,
  0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f,
  0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00,
  0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00,
  0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00,
  0x7f, 0x00, 0xfe, 0x03, 0xc0, 0x7f, 0x00, 0xfe,
  0x0f, 0xf0, 0x7f, 0x00, 0xfe, 0x0f, 0xf0, 0x7f,
  0x00, 0xfe, 0x0f, 0xf0, 0x7f, 0x00, 0xfe, 0x0f,
  0xf0, 0x7f, 0x00, 0xfe, 0x0f, 0xf0, 0x7f, 0x00,
  0xfe, 0x0f, 0xf0, 0x7f, 0x00, 0xfe, 0x0f, 0xf0,
  0x7f, 0x00, 0xfe, 0x0f, 0xf0, 0x7f, 0x00, 0xfe,
  0x0f, 0xf0, 0x7f, 0x00, 0xfe, 0x0f, 0xf0, 0x7f,
  0x00, 0xfe, 0x03, 0xc0, 0x7f, 0x00, 0xfe, 0x00,
  0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00,
  0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00,
  0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe,
  0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f,
  0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00,
  0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00,
  0x7e, 0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00,
  0xfe, 0x00, 0x7f, 0x00, 0x00, 0xfe, 0x00, 0x7f,
  0x00, 0x00, 0xfe, 0x00, 0x3f, 0x00, 0x00, 0xfc,
  0x00, 0x3f, 0x80, 0x01, 0xfc, 0x00, 0x3f, 0x80,
  0x01, 0xfc, 0x00, 0x1f, 0xc0, 0x03, 0xf8, 0x00,
  0x1f, 0xe0, 0x07, 0xf8, 0x00, 0x0f, 0xf8, 0x1f,
  0xf0, 0x00, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x07,
  0xff, 0xff, 0xc0, 0x00, 0x03, 0xff, 0xff, 0xc0,
  0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x7f,
  0xfe, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00,
},
{
  0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x03, 0xfc,
  0x00, 0x00, 0x00, 0x03, 0xfc, 0x00, 0x00, 0x00,
  0x07, 0xfc, 0x00, 0x00, 0x00, 0x0f, 0xfc, 0x00,
  0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x00, 0x7f,
  0xfc, 0x00, 0x00, 0x00, 0xff, 0xfc, 0x00, 0x00,
  0x07, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xf9, 0xfc,
  0x00, 0x00, 0x7f, 0xf9, 0xfc, 0x00, 0x00, 0x7f,
  0xf1, 0xfc, 0x00, 0x00, 0x7f, 0xe1, 0xfc, 0x00,
  0x00, 0x7f, 0xc1, 0xfc, 0x00, 0x00, 0x7f, 0x81,
  0xfc, 0x00, 0x00, 0x7e, 0x01, 0xfc, 0x00, 0x00,
  0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc,
  0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00,
  0x01, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00,
  0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x01,
  0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00,
  0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc,
  0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00,
  0x01, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00,
  0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x01,
  0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00,
  0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc,
  0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00,
  0x01, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00,
  0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x01,
  0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00,
  0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc,
  0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00,
  0x01, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00,
  0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x01,
  0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00,
  0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc,
  0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00,
  0x01, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00,
  0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x01,
  0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00,
  0x00, 0x01, 0xfc, 0x00, 0x00, 0x3f, 0xff, 0xff,
  0xff, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0xc0, 0x3f,
  0xff, 0xff, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xff,
  0xc0, 0x3f, 0xff, 0xff, 0xff, 0xc0, 0x3f, 0xff,
  0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
},
{
  0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0xff, 0xff,
  0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x07,
  0xff, 0xff, 0xe0, 0x00, 0x0f, 0xff, 0xff, 0xf0,
  0x00, 0x0f, 0xff, 0xff, 0xf8, 0x00, 0x1f, 0xf8,
  0x1f, 0xfc, 0x00, 0x1f, 0xe0, 0x07, 0xfc, 0x00,
  0x3f, 0xc0, 0x01, 0xfe, 0x00, 0x3f, 0x80, 0x01,
  0xfe, 0x00, 0x3f, 0x80, 0x00, 0xfe, 0x00, 0x7f,
  0x80, 0x00, 0xfe, 0x00, 0x7f, 0x00, 0x00, 0xfe,
  0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x7f, 0x00,
  0x00, 0xff, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00,
  0x7f, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00,
  0xff, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x1f,
  0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00,
  0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x01,
  0xfe, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00,
  0x00, 0x03, 0xfc, 0x00, 0x00, 0x00, 0x0f, 0xfc,
  0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x01,
  0xff, 0xf0, 0x00, 0x00, 0x07, 0xff, 0xe0, 0x00,
  0x00, 0x1f, 0xff, 0xc0, 0x00, 0x00, 0xff, 0xff,
  0x00, 0x00, 0x01, 0xff, 0xfc, 0x00, 0x00, 0x07,
  0xff, 0xf0, 0x00, 0x00, 0x0f, 0xff, 0xc0, 0x00,
  0x00, 0x1f, 0xff, 0x00, 0x00, 0x00, 0x3f, 0xf8,
  0x00, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00,
  0x7f, 0xc0, 0x00, 0x00, 0x00, 0x7f, 0x80, 0x00,
  0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xff,
  0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00,
  0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00,
  0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0xfe, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xfe,
  0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00,
  0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00,
  0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
  0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff,
  0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff,
  0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},
{
  0x00, 0x1f, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xfe,
  0x00, 0x00, 0x03, 0xff, 0xff, 0x80, 0x00, 0x07,
  0xff, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0xe0,
  0x00, 0x1f, 0xff, 0xff, 0xf0, 0x00, 0x1f, 0xf0,
  0x1f, 0xf8, 0x00, 0x3f, 0xe0, 0x07, 0xf8, 0x00,
  0x3f, 0xc0, 0x03, 0xfc, 0x00, 0x7f, 0x80, 0x01,
  0xfc, 0x00, 0x7f, 0x80, 0x01, 0xfc, 0x00, 0x7f,
  0x00, 0x00, 0xfe, 0x00, 0x7f, 0x00, 0x00, 0xfe,
  0x00, 0x7f, 0x00, 0x00, 0xfe, 0x00, 0xff, 0x00,
  0x00, 0xfe, 0x00, 0xff, 0x00, 0x00, 0xfe, 0x00,
  0x1f, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0xfe, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xfe,
  0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00,
  0x01, 0xfc, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x00,
  0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x03,
  0xf8, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00,
  0x3f, 0xff, 0xc0, 0x00, 0x00, 0x3f, 0xfe, 0x00,
  0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x3f,
  0xfc, 0x00, 0x00, 0x00, 0x3f, 0xff, 0x00, 0x00,
  0x00, 0x3f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x0f,
  0xf0, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00,
  0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xfc,
  0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00,
  0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00,
  0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f,
  0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x3c, 0x00,
  0x00, 0x7f, 0x00, 0xfc, 0x00, 0x00, 0x7f, 0x00,
  0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00,
  0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe,
  0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0xff,
  0x00, 0xff, 0x00, 0x00, 0xfe, 0x00, 0xff, 0x00,
  0x00, 0xfe, 0x00, 0x7f, 0x80, 0x01, 0xfe, 0x00,
  0x7f, 0xc0, 0x03, 0xfc, 0x00, 0x3f, 0xf0, 0x1f,
  0xfc, 0x00, 0x3f, 0xff, 0xff, 0xf8, 0x00, 0x1f,
  0xff, 0xff, 0xf0, 0x00, 0x0f, 0xff, 0xff, 0xe0,
  0x00, 0x07, 0xff, 0xff, 0xc0, 0x00, 0x01, 0xff,
  0xff, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x00,
},
{
  0x00, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x3f,
  0xf0, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00,
  0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xf0,
  0x00, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x01,
  0xff, 0xf0, 0x00, 0x00, 0x01, 0xff, 0xf0, 0x00,
  0x00, 0x03, 0xff, 0xf0, 0x00, 0x00, 0x03, 0xff,
  0xf0, 0x00, 0x00, 0x07, 0xff, 0xf0, 0x00, 0x00,
  0x07, 0xf7, 0xf0, 0x00, 0x00, 0x0f, 0xf7, 0xf0,
  0x00, 0x00, 0x0f, 0xe7, 0xf0, 0x00, 0x00, 0x1f,
  0xe7, 0xf0, 0x00, 0x00, 0x1f, 0xc7, 0xf0, 0x00,
  0x00, 0x3f, 0xc7, 0xf0, 0x00, 0x00, 0x3f, 0x87,
  0xf0, 0x00, 0x00, 0x7f, 0x87, 0xf0, 0x00, 0x00,
  0x7f, 0x07, 0xf0, 0x00, 0x00, 0xff, 0x07, 0xf0,
  0x00, 0x00, 0xfe, 0x07, 0xf0, 0x00, 0x01, 0xfe,
  0x07, 0xf0, 0x00, 0x01, 0xfc, 0x07, 0xf0, 0x00,
  0x01, 0xfc, 0x07, 0xf0, 0x00, 0x03, 0xf8, 0x07,
  0xf0, 0x00, 0x03, 0xf8, 0x07, 0xf0, 0x00, 0x07,
  0xf8, 0x07, 0xf0, 0x00, 0x07, 0xf0, 0x07, 0xf0,
  0x00, 0x0f, 0xf0, 0x07, 0xf0, 0x00, 0x0f, 0xe0,
  0x07, 0xf0, 0x00, 0x1f, 0xe0, 0x07, 0xf0, 0x00,
  0x1f, 0xc0, 0x07, 0xf0, 0x00, 0x3f, 0xc0, 0x07,
  0xf0, 0x00, 0x3f, 0x80, 0x07, 0xf0, 0x00, 0x7f,
  0x80, 0x07, 0xf0, 0x00, 0x7f, 0x00, 0x07, 0xf0,
  0x00, 0xff, 0x00, 0x07, 0xf0, 0x00, 0xfe, 0x00,
  0x07, 0xf0, 0x00, 0xfe, 0x00, 0x07, 0xf0, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff,
  0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff,
  0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff,
  0xe0, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00,
  0x07, 0xf0, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00,
  0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x07,
  0xf0, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00,
  0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x07, 0xf0,
  0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00,
  0x07, 0xf0, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00,
  0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x07,
  0xf0, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00,
  0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x07, 0xf0,
  0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00,
  0x07, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},
{
  0x7f, 0xff, 0xff, 0xfc, 0x00, 0x7f, 0xff, 0xff,
  0xfc, 0x00, 0x7f, 0xff, 0xff, 0xfc, 0x00, 0x7f,
  0xff, 0xff, 0xfc, 0x00, 0x7f, 0xff, 0xff, 0xfc,
  0x00, 0x7f, 0xff, 0xff, 0xfc, 0x00, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00,
  0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f,
  0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00,
  0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00,
  0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f,
  0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00,
  0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00,
  0x7f, 0xff, 0xfc, 0x00, 0x00, 0x7f, 0xff, 0xff,
  0x80, 0x00, 0x7f, 0xff, 0xff, 0xe0, 0x00, 0x7f,
  0xff, 0xff, 0xf0, 0x00, 0x7f, 0xff, 0xff, 0xf8,
  0x00, 0x7f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00,
  0x0f, 0xfc, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x00,
  0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0xfe, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f,
  0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00,
  0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00,
  0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f,
  0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0xfc, 0x00,
  0x00, 0x7f, 0x00, 0xfc, 0x00, 0x00, 0x7f, 0x00,
  0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00,
  0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe,
  0x00, 0x00, 0xff, 0x00, 0xfe, 0x00, 0x00, 0xff,
  0x00, 0xfe, 0x00, 0x00, 0xfe, 0x00, 0xff, 0x00,
  0x01, 0xfe, 0x00, 0xff, 0x00, 0x01, 0xfe, 0x00,
  0x7f, 0x80, 0x07, 0xfc, 0x00, 0x7f, 0xe0, 0x1f,
  0xf8, 0x00, 0x3f, 0xff, 0xff, 0xf8, 0x00, 0x3f,
  0xff, 0xff, 0xf0, 0x00, 0x1f, 0xff, 0xff, 0xe0,
  0x00, 0x0f, 0xff, 0xff, 0xc0, 0x00, 0x03, 0xff,
  0xff, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00,
},
{
  0x00, 0x0f, 0xf8, 0x00, 0x00, 0x00, 0x7f, 0xff,
  0x80, 0x00, 0x01, 0xff, 0xff, 0xc0, 0x00, 0x03,
  0xff, 0xff, 0xf0, 0x00, 0x07, 0xff, 0xff, 0xf0,
  0x00, 0x0f, 0xff, 0xff, 0xf8, 0x00, 0x1f, 0xf8,
  0x1f, 0xfc, 0x00, 0x3f, 0xe0, 0x07, 0xfc, 0x00,
  0x3f, 0xc0, 0x03, 0xfc, 0x00, 0x3f, 0x80, 0x01,
  0xfe, 0x00, 0x7f, 0x00, 0x01, 0xfe, 0x00, 0x7f,
  0x00, 0x01, 0xfe, 0x00, 0x7f, 0x00, 0x00, 0xff,
  0x00, 0xff, 0x00, 0x00, 0xfe, 0x00, 0xfe, 0x00,
  0x00, 0xc0, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0xfe, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00,
  0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xfe,
  0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00,
  0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00,
  0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0xfe, 0x03, 0xfc, 0x00, 0x00, 0xfe, 0x0f, 0xff,
  0x80, 0x00, 0xfe, 0x3f, 0xff, 0xc0, 0x00, 0xfe,
  0x7f, 0xff, 0xe0, 0x00, 0xfe, 0x7f, 0xff, 0xf0,
  0x00, 0xfe, 0xff, 0xff, 0xf8, 0x00, 0xff, 0xf8,
  0x1f, 0xfc, 0x00, 0xff, 0xe0, 0x07, 0xfc, 0x00,
  0xff, 0xc0, 0x03, 0xfc, 0x00, 0xff, 0x80, 0x01,
  0xfe, 0x00, 0xff, 0x00, 0x01, 0xfe, 0x00, 0xff,
  0x00, 0x00, 0xfe, 0x00, 0xff, 0x00, 0x00, 0xff,
  0x00, 0xfe, 0x00, 0x00, 0xff, 0x00, 0xfe, 0x00,
  0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00,
  0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00,
  0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe,
  0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f,
  0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00,
  0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00,
  0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00,
  0x7f, 0x00, 0x7e, 0x00, 0x00, 0xff, 0x00, 0x7f,
  0x00, 0x00, 0xff, 0x00, 0x7f, 0x00, 0x00, 0xfe,
  0x00, 0x7f, 0x00, 0x01, 0xfe, 0x00, 0x3f, 0x80,
  0x01, 0xfe, 0x00, 0x3f, 0xc0, 0x03, 0xfc, 0x00,
  0x1f, 0xe0, 0x07, 0xfc, 0x00, 0x1f, 0xf8, 0x1f,
  0xf8, 0x00, 0x0f, 0xff, 0xff, 0xf8, 0x00, 0x07,
  0xff, 0xff, 0xf0, 0x00, 0x03, 0xff, 0xff, 0xe0,
  0x00, 0x01, 0xff, 0xff, 0x80, 0x00, 0x00, 0x7f,
  0xff, 0x00, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x00,
},
{
  0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
  0xff, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff,
  0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff,
  0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff,
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  0xfe, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00,
  0x00, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x03, 0xfc,
  0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00,
  0x03, 0xf8, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x00,
  0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x0f,
  0xf0, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00,
  0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0xe0,
  0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00,
  0x3f, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00,
  0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x7f,
  0x80, 0x00, 0x00, 0x00, 0x7f, 0x80, 0x00, 0x00,
  0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
  0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00,
  0xfe, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00,
  0x00, 0x01, 0xfe, 0x00, 0x00, 0x00, 0x01, 0xfe,
  0x00, 0x00, 0x00, 0x01, 0xfc, 0x00, 0x00, 0x00,
  0x01, 0xfc, 0x00, 0x00, 0x00, 0x03, 0xfc, 0x00,
  0x00, 0x00, 0x03, 0xfc, 0x00, 0x00, 0x00, 0x03,
  0xf8, 0x00, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00,
  0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x07, 0xf8,
  0x00, 0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00,
  0x07, 0xf0, 0x00, 0x00, 0x00, 0x07, 0xf0, 0x00,
  0x00, 0x00, 0x07, 0xf0, 0x00, 0x00, 0x00, 0x0f,
  0xf0, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00,
  0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x0f, 0xe0,
  0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00,
  0x1f, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x00,
  0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x1f,
  0xc0, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00, 0x00,
  0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0xc0,
  0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00, 0x00, 0x00,
  0x3f, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x00,
  0x00, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x3f,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
},
{
  0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0xff, 0xff,
  0x80, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x07,
  0xff, 0xff, 0xe0, 0x00, 0x0f, 0xff, 0xff, 0xf0,
  0x00, 0x1f, 0xff, 0xff, 0xf8, 0x00, 0x3f, 0xf0,
  0x0f, 0xfc, 0x00, 0x3f, 0xc0, 0x03, 0xfc, 0x00,
  0x3f, 0x80, 0x01, 0xfc, 0x00, 0x7f, 0x80, 0x01,
  0xfe, 0x00, 0x7f, 0x80, 0x01, 0xfe, 0x00, 0x7f,
  0x00, 0x00, 0xfe, 0x00, 0x7f, 0x00, 0x00, 0xfe,
  0x00, 0x7f, 0x00, 0x00, 0xfe, 0x00, 0x7f, 0x00,
  0x00, 0xfe, 0x00, 0x7f, 0x00, 0x00, 0xfe, 0x00,
  0x7f, 0x00, 0x00, 0xfe, 0x00, 0x7f, 0x00, 0x00,
  0xfe, 0x00, 0x7f, 0x00, 0x00, 0xfe, 0x00, 0x7f,
  0x00, 0x00, 0xfe, 0x00, 0x7f, 0x00, 0x00, 0xfe,
  0x00, 0x7f, 0x00, 0x00, 0xfe, 0x00, 0x3f, 0x80,
  0x01, 0xfc, 0x00, 0x3f, 0x80, 0x01, 0xfc, 0x00,
  0x1f, 0xc0, 0x03, 0xf8, 0x00, 0x1f, 0xc0, 0x03,
  0xf8, 0x00, 0x0f, 0xf0, 0x0f, 0xf0, 0x00, 0x03,
  0xff, 0xff, 0xc0, 0x00, 0x00, 0xff, 0xff, 0x00,
  0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x7f,
  0xfe, 0x00, 0x00, 0x01, 0xff, 0xff, 0x80, 0x00,
  0x07, 0xff, 0xff, 0xe0, 0x00, 0x0f, 0xf0, 0x0f,
  0xf0, 0x00, 0x1f, 0xc0, 0x03, 0xf8, 0x00, 0x3f,
  0x80, 0x01, 0xfc, 0x00, 0x3f, 0x00, 0x00, 0xfc,
  0x00, 0x7f, 0x00, 0x00, 0xfe, 0x00, 0x7f, 0x00,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0x00, 0x7e, 0x00,
  0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00,
  0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe,
  0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f,
  0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00,
  0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00,
  0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00,
  0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe,
  0x00, 0x00, 0x7f, 0x00, 0xff, 0x00, 0x00, 0xff,
  0x00, 0xff, 0x00, 0x00, 0xff, 0x00, 0x7f, 0x00,
  0x00, 0xfe, 0x00, 0x7f, 0x80, 0x01, 0xfe, 0x00,
  0x7f, 0xc0, 0x03, 0xfe, 0x00, 0x3f, 0xf0, 0x0f,
  0xfc, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 0x1f,
  0xff, 0xff, 0xf8, 0x00, 0x0f, 0xff, 0xff, 0xf0,
  0x00, 0x07, 0xff, 0xff, 0xe0, 0x00, 0x01, 0xff,
  0xff, 0x80, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00,
},
{
  0x00, 0x1f, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xfe,
  0x00, 0x00, 0x01, 0xff, 0xff, 0x80, 0x00, 0x07,
  0xff, 0xff, 0xc0, 0x00, 0x0f, 0xff, 0xff, 0xe0,
  0x00, 0x1f, 0xff, 0xff, 0xf0, 0x00, 0x1f, 0xf8,
  0x1f, 0xf8, 0x00, 0x3f, 0xe0, 0x07, 0xf8, 0x00,
  0x3f, 0xc0, 0x03, 0xfc, 0x00, 0x7f, 0x80, 0x01,
  0xfc, 0x00, 0x7f, 0x00, 0x00, 0xfe, 0x00, 0x7f,
  0x00, 0x00, 0xfe, 0x00, 0xff, 0x00, 0x00, 0xfe,
  0x00, 0xff, 0x00, 0x00, 0x7e, 0x00, 0xfe, 0x00,
  0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00,
  0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00,
  0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe,
  0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f,
  0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00,
  0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00, 0x7f, 0x00,
  0xfe, 0x00, 0x00, 0x7f, 0x00, 0xfe, 0x00, 0x00,
  0x7f, 0x00, 0xff, 0x00, 0x00, 0x7f, 0x00, 0xff,
  0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0xff,
  0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0x7f, 0x80,
  0x01, 0xff, 0x00, 0x7f, 0xc0, 0x03, 0xff, 0x00,
  0x3f, 0xe0, 0x07, 0xff, 0x00, 0x3f, 0xf8, 0x1f,
  0xff, 0x00, 0x1f, 0xff, 0xff, 0x7f, 0x00, 0x0f,
  0xff, 0xfe, 0x7f, 0x00, 0x0f, 0xff, 0xfe, 0x7f,
  0x00, 0x03, 0xff, 0xfc, 0x7f, 0x00, 0x01, 0xff,
  0xf0, 0x7f, 0x00, 0x00, 0x3f, 0xc0, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00,
  0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f,
  0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00,
  0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x7f, 0x00, 0x03, 0x00, 0x00,
  0x7f, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x00, 0xff,
  0x00, 0x00, 0xfe, 0x00, 0x7f, 0x80, 0x00, 0xfe,
  0x00, 0x7f, 0x80, 0x00, 0xfe, 0x00, 0x7f, 0x80,
  0x01, 0xfc, 0x00, 0x3f, 0xc0, 0x03, 0xfc, 0x00,
  0x3f, 0xe0, 0x07, 0xf8, 0x00, 0x3f, 0xf8, 0x1f,
  0xf8, 0x00, 0x1f, 0xff, 0xff, 0xf0, 0x00, 0x0f,
  0xff, 0xff, 0xe0, 0x00, 0x07, 0xff, 0xff, 0xc0,
  0x00, 0x03, 0xff, 0xff, 0x80, 0x00, 0x01, 0xff,
  0xfe, 0x00, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00,
}
};

const PROGMEM uint8_t DIGITS_18_32[10][96] = {
{
  0x03, 0xe0, 0x00, 0x0f, 0xf8, 0x00, 0x1f, 0xfc,
  0x00, 0x3c, 0x1e, 0x00, 0x38, 0x0e, 0x00, 0x78,
  0x0f, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00,
  0x70, 0x07, 0x00, 0xf0, 0x07, 0x80, 0xf0, 0x07,
  0x80, 0xf0, 0x07, 0x80, 0xf0, 0x07, 0x80, 0xf1,
  0xc7, 0x80, 0xf3, 0xe7, 0x80, 0xf3, 0xe7, 0x80,
  0xf3, 0xe7, 0x80, 0xf3, 0xe7, 0x80, 0xf1, 0xc7,
  0x80, 0xf0, 0x07, 0x80, 0xf0, 0x07, 0x80, 0xf0,
  0x07, 0x80, 0xf0, 0x07, 0x80, 0x70, 0x07, 0x00,
  0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x78, 0x0f,
  0x00, 0x38, 0x0e, 0x00, 0x3c, 0x1e, 0x00, 0x1f,
  0xfc, 0x00, 0x0f, 0xf8, 0x00, 0x03, 0xe0, 0x00,
},
{
  0x00, 0xf0, 0x00, 0x01, 0xf0, 0x00, 0x03, 0xf0,
  0x00, 0x0e, 0xf0, 0x00, 0x7e, 0xf0, 0x00, 0x7c,
  0xf0, 0x00, 0x70, 0xf0, 0x00, 0x00, 0xf0, 0x00,
  0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0,
  0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00,
  0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00,
  0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0,
  0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00,
  0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00,
  0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0,
  0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x3f,
  0xff, 0xc0, 0x3f, 0xff, 0xc0, 0x3f, 0xff, 0xc0,
},
{
  0x07, 0xf0, 0x00, 0x1f, 0xfc, 0x00, 0x3f, 0xfe,
  0x00, 0x3c, 0x1f, 0x00, 0x78, 0x0f, 0x00, 0x78,
  0x0f, 0x00, 0x78, 0x07, 0x00, 0x70, 0x07, 0x00,
  0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x00, 0x07,
  0x00, 0x00, 0x07, 0x00, 0x00, 0x0f, 0x00, 0x00,
  0x0f, 0x00, 0x00, 0x1f, 0x00, 0x00, 0xfe, 0x00,
  0x03, 0xfc, 0x00, 0x0f, 0xf0, 0x00, 0x3f, 0x80,
  0x00, 0x3e, 0x00, 0x00, 0x78, 0x00, 0x00, 0x78,
  0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00,
  0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00,
  0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xff,
  0xff, 0x80, 0xff, 0xff, 0x80, 0xff, 0xff, 0x80,
},
{
  0x07, 0xf0, 0x00, 0x1f, 0xfc, 0x00, 0x3f, 0xfe,
  0x00, 0x7c, 0x1e, 0x00, 0x78, 0x0f, 0x00, 0x78,
  0x0f, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00,
  0x00, 0x07, 0x00, 0x00, 0x07, 0x00, 0x00, 0x0f,
  0x00, 0x00, 0x0f, 0x00, 0x00, 0x1e, 0x00, 0x07,
  0xf8, 0x00, 0x07, 0xf0, 0x00, 0x07, 0xf8, 0x00,
  0x00, 0x1e, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x07,
  0x00, 0x00, 0x07, 0x00, 0x00, 0x07, 0x80, 0x00,
  0x07, 0x80, 0x00, 0x07, 0x80, 0xf0, 0x07, 0x80,
  0xf0, 0x07, 0x80, 0xf0, 0x07, 0x80, 0xf0, 0x07,
  0x00, 0x78, 0x0f, 0x00, 0x7c, 0x1f, 0x00, 0x3f,
  0xfe, 0x00, 0x1f, 0xfc, 0x00, 0x0f, 0xf0, 0x00,
},
{
  0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00, 0x00, 0xfe,
  0x00, 0x00, 0xfe, 0x00, 0x01, 0xfe, 0x00, 0x01,
  0xfe, 0x00, 0x03, 0xde, 0x00, 0x03, 0xde, 0x00,
  0x07, 0x9e, 0x00, 0x07, 0x9e, 0x00, 0x0f, 0x1e,
  0x00, 0x0f, 0x1e, 0x00, 0x1e, 0x1e, 0x00, 0x1e,
  0x1e, 0x00, 0x1c, 0x1e, 0x00, 0x3c, 0x1e, 0x00,
  0x38, 0x1e, 0x00, 0x78, 0x1e, 0x00, 0x70, 0x1e,
  0x00, 0xf0, 0x1e, 0x00, 0xff, 0xff, 0xc0, 0xff,
  0xff, 0xc0, 0xff, 0xff, 0xc0, 0x00, 0x1e, 0x00,
  0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e,
  0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00,
  0x1e, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00,
},
{
  0x7f, 0xff, 0x00, 0x7f, 0xff, 0x00, 0x7f, 0xff,
  0x00, 0x70, 0x00, 0x00, 0x70, 0x00, 0x00, 0x70,
  0x00, 0x00, 0x70, 0x00, 0x00, 0x70, 0x00, 0x00,
  0x70, 0x00, 0x00, 0x70, 0x00, 0x00, 0x70, 0x00,
  0x00, 0x70, 0x00, 0x00, 0x7f, 0xf8, 0x00, 0x7f,
  0xfc, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x1f, 0x00,
  0x00, 0x0f, 0x00, 0x00, 0x07, 0x00, 0x00, 0x07,
  0x80, 0x00, 0x07, 0x80, 0x00, 0x07, 0x80, 0x00,
  0x07, 0x80, 0x00, 0x07, 0x80, 0xf0, 0x07, 0x80,
  0xf0, 0x07, 0x80, 0xf0, 0x07, 0x80, 0xf0, 0x07,
  0x00, 0xf0, 0x0f, 0x00, 0x7c, 0x1f, 0x00, 0x7f,
  0xfe, 0x00, 0x3f, 0xfc, 0x00, 0x0f, 0xf0, 0x00,
},
{
  0x07, 0xf0, 0x00, 0x1f, 0xfc, 0x00, 0x3f, 0xfe,
  0x00, 0x3c, 0x1e, 0x00, 0x78, 0x0f, 0x00, 0x78,
  0x0f, 0x00, 0x70, 0x0f, 0x00, 0xf0, 0x07, 0x00,
  0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00,
  0x00, 0xf0, 0x00, 0x00, 0xf1, 0xf0, 0x00, 0xf7,
  0xfc, 0x00, 0xf7, 0xfe, 0x00, 0xfc, 0x1f, 0x00,
  0xf8, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x07,
  0x80, 0xf0, 0x07, 0x80, 0xf0, 0x07, 0x80, 0xf0,
  0x07, 0x80, 0xf0, 0x07, 0x80, 0xf0, 0x07, 0x80,
  0xf0, 0x07, 0x80, 0x70, 0x07, 0x00, 0x70, 0x0f,
  0x00, 0x78, 0x0f, 0x00, 0x3c, 0x1e, 0x00, 0x1f,
  0xfe, 0x00, 0x0f, 0xfc, 0x00, 0x07, 0xf0, 0x00,
},
{
  0xff, 0xff, 0x80, 0xff, 0xff, 0x80, 0xff, 0xff,
  0x80, 0x00, 0x07, 0x80, 0x00, 0x0f, 0x00, 0x00,
  0x0f, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x1e, 0x00,
  0x00, 0x1e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c,
  0x00, 0x00, 0x38, 0x00, 0x00, 0x78, 0x00, 0x00,
  0x78, 0x00, 0x00, 0x70, 0x00, 0x00, 0xf0, 0x00,
  0x00, 0xf0, 0x00, 0x00, 0xf0, 0x00, 0x00, 0xe0,
  0x00, 0x01, 0xe0, 0x00, 0x01, 0xe0, 0x00, 0x01,
  0xe0, 0x00, 0x01, 0xe0, 0x00, 0x01, 0xc0, 0x00,
  0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0,
  0x00, 0x03, 0xc0, 0x00, 0x03, 0xc0, 0x00, 0x03,
  0x80, 0x00, 0x07, 0x80, 0x00, 0x07, 0x80, 0x00,
},
{
  0x07, 0xf0, 0x00, 0x1f, 0xfc, 0x00, 0x3f, 0xfe,
  0x00, 0x7c, 0x1e, 0x00, 0x78, 0x0f, 0x00, 0x78,
  0x0f, 0x00, 0x78, 0x0f, 0x00, 0x70, 0x07, 0x00,
  0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x78, 0x0f,
  0x00, 0x78, 0x0f, 0x00, 0x38, 0x0e, 0x00, 0x3c,
  0x1e, 0x00, 0x0f, 0xf8, 0x00, 0x07, 0xf0, 0x00,
  0x1f, 0xfc, 0x00, 0x3c, 0x1e, 0x00, 0x78, 0x0f,
  0x00, 0x70, 0x07, 0x00, 0xf0, 0x07, 0x80, 0xf0,
  0x07, 0x80, 0xf0, 0x07, 0x80, 0xf0, 0x07, 0x80,
  0xf0, 0x07, 0x80, 0xf0, 0x07, 0x80, 0xf0, 0x07,
  0x80, 0x78, 0x0f, 0x00, 0x7c, 0x1f, 0x00, 0x3f,
  0xfe, 0x00, 0x1f, 0xfc, 0x00, 0x07, 0xf0, 0x00,
},
{
  0x07, 0xf0, 0x00, 0x1f, 0xf8, 0x00, 0x3f, 0xfc,
  0x00, 0x3c, 0x1e, 0x00, 0x78, 0x0f, 0x00, 0x78,
  0x07, 0x00, 0xf0, 0x07, 0x00, 0xf0, 0x07, 0x80,
  0xf0, 0x07, 0x80, 0xf0, 0x07, 0x80, 0xf0, 0x07,
  0x80, 0xf0, 0x07, 0x80, 0xf0, 0x07, 0x80, 0xf0,
  0x07, 0x80, 0x78, 0x07, 0x80, 0x78, 0x0f, 0x80,
  0x7c, 0x1f, 0x80, 0x3f, 0xf7, 0x80, 0x1f, 0xf7,
  0x80, 0x07, 0xc7, 0x80, 0x00, 0x07, 0x80, 0x00,
  0x07, 0x80, 0x00, 0x07, 0x80, 0x00, 0x07, 0x80,
  0x30, 0x07, 0x80, 0xf8, 0x07, 0x00, 0x78, 0x0f,
  0x00, 0x78, 0x0f, 0x00, 0x3c, 0x1e, 0x00, 0x3f,
  0xfe, 0x00, 0x1f, 0xf8, 0x00, 0x07, 0xf0, 0x00,
}
};

const PROGMEM uint8_t ICON_WARNING_22_22[66] = {
  0x00, 0x00, 0x00, 0x00, 0x30, 0x00,
  0x00, 0x78, 0x00, 0x00, 0x78, 0x00,
  0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00,
  0x01, 0x86, 0x00, 0x03, 0x87, 0x00,
  0x03, 0x87, 0x00, 0x07, 0x87, 0x80,
  0x07, 0x87, 0x80, 0x0f, 0x87, 0xc0,
  0x0f, 0xcf, 0xc0, 0x1f, 0xcf, 0xe0,
  0x3f, 0xcf, 0xf0, 0x3f, 0xff, 0xf0,
  0x7f, 0xcf, 0xf8, 0x7f, 0x87, 0xf8,
  0xff, 0x87, 0xfc, 0xff, 0xcf, 0xfc,
  0xff, 0xff, 0xfc, 0x7f, 0xff, 0xf8,
};

const PROGMEM uint8_t ICON_ARROW_UP_11_6[12] = {
  0x04, 0x00, 0x0e, 0x00, 0x1f, 0x00,
  0x3f, 0x80, 0x7f, 0xc0, 0xff, 0xe0,
};

const PROGMEM uint8_t ICON_ARROW_DOWN_11_6[12] = {
  0xff, 0xe0, 0x7f, 0xc0, 0x3f, 0x80,
  0x1f, 0x00, 0x0e, 0x00, 0x04, 0x00,
};

const PROGMEM uint8_t PICO_FONT_4_6[384] = {
  0xe0, 0xc0, 0xe0, 0xe0, 0xa0, 0xe0, 0x80, 0xe0,
  0xe0, 0xe0, 0xe0, 0xe0, 0x60, 0xc0, 0xe0, 0xe0,
  0x60, 0xa0, 0xe0, 0xe0, 0xa0, 0x80, 0xe0, 0xc0,
  0x60, 0xe0, 0x40, 0xe0, 0x60, 0xe0, 0xa0, 0xa0,
  0xa0, 0xa0, 0xa0, 0xe0, 0x80, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x40, 0x40,
  0xa0, 0x40, 0x20, 0x20, 0xa0, 0x80, 0x80, 0x20,
  0xa0, 0xa0, 0xa0, 0xa0, 0x80, 0xa0, 0x80, 0x80,
  0x80, 0xa0, 0x40, 0x40, 0xa0, 0x80, 0xe0, 0xa0,
  0xa0, 0xa0, 0xa0, 0xa0, 0x80, 0x40, 0xa0, 0xa0,
  0xa0, 0xa0, 0xa0, 0x20, 0x80, 0x20, 0x40, 0x00,
  0x00, 0x00, 0x40, 0x40, 0x40, 0x00, 0x80, 0x20,
  0xa0, 0x40, 0xe0, 0x60, 0xe0, 0xe0, 0xe0, 0x20,
  0xe0, 0xe0, 0xe0, 0xc0, 0x80, 0xa0, 0xc0, 0xc0,
  0x80, 0xe0, 0x40, 0x40, 0xc0, 0x80, 0xa0, 0xa0,
  0xa0, 0xe0, 0xa0, 0xc0, 0xe0, 0x40, 0xa0, 0xa0,
  0xa0, 0x40, 0xe0, 0x40, 0x80, 0x60, 0xe0, 0x00,
  0xe0, 0x00, 0x40, 0x00, 0x00, 0x00, 0x80, 0x20,
  0xa0, 0x40, 0x80, 0x20, 0x20, 0x20, 0xa0, 0x20,
  0xa0, 0x20, 0xa0, 0xa0, 0x80, 0xa0, 0x80, 0x80,
  0xa0, 0xa0, 0x40, 0x40, 0xa0, 0x80, 0xa0, 0xa0,
  0xa0, 0x80, 0xc0, 0xa0, 0x20, 0x40, 0xa0, 0xe0,
  0xe0, 0xa0, 0x20, 0x80, 0x00, 0x00, 0x40, 0x40,
  0x00, 0x00, 0x40, 0x40, 0x40, 0x00, 0x80, 0x20,
  0xe0, 0xe0, 0xe0, 0xe0, 0x20, 0xe0, 0xe0, 0x20,
  0xe0, 0x20, 0xa0, 0xe0, 0x60, 0xe0, 0xe0, 0x80,
  0xe0, 0xa0, 0xe0, 0xc0, 0xa0, 0xe0, 0xa0, 0xa0,
  0xc0, 0x80, 0x60, 0xa0, 0xc0, 0x40, 0x60, 0x40,
  0xe0, 0xa0, 0xe0, 0xe0, 0x80, 0x40, 0x00, 0x80,
  0x00, 0x40, 0x80, 0x00, 0x80, 0xe0, 0x40, 0x40,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const PROGMEM uint8_t ICON_SMALL_ARROW_UP_6_3[3] = {
    B0010000,
    B0111000,
    B1111100,
};

const PROGMEM uint8_t ICON_SMALL_ARROW_DOWN_6_3[3] = {
    B1111100,
    B0111000,
    B0010000,
};

const PROGMEM uint8_t ICON_SMALL_ARROW_EQ_6_3[3] = {
    B0111000,
    B0111000,
    B0111000,
};
