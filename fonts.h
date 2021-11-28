#pragma once

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "display.h"

class FontPico {
public:
    FontPico(Display& display);

    void draw(const char* s, uint8_t x, uint8_t y);
    void draw_char(char c, uint8_t x, uint8_t y);

private:
    Display& m_display;
    static constexpr size_t m_image_width{128};
};

extern const PROGMEM uint8_t DIGITS_36_64[10][320];
extern const PROGMEM uint8_t DIGITS_18_32[10][96];
extern const PROGMEM uint8_t PICO_FONT_4_6[384];
extern const PROGMEM uint8_t ICON_WARNING_22_22[66];
extern const PROGMEM uint8_t ICON_ARROW_UP_11_6[12];
extern const PROGMEM uint8_t ICON_ARROW_DOWN_11_6[12];
extern const PROGMEM uint8_t ICON_SMALL_ARROW_UP_6_3[3];
extern const PROGMEM uint8_t ICON_SMALL_ARROW_DOWN_6_3[3];
extern const PROGMEM uint8_t ICON_SMALL_ARROW_EQ_6_5[5];
