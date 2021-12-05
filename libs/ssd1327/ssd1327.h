#pragma once

#include <Arduino.h>
#include "display.h"

class Ssd1327 : public Display {
public:
    Ssd1327(byte rst=9, byte dc=8, byte cs=10, byte din=11, byte clk=13);

    void begin() final;
    void clear() final;
    void flush() final;
    void draw_pixel(uint8_t x, uint8_t y) final;
    void draw_bitmap(uint8_t x, uint8_t y, Bitmap&& bitmap) final;

private:
    void command(uint8_t cmd);

    const byte m_rst;
    const byte m_dc;
    const byte m_cs;
    const byte m_din;
    const byte m_clk;
    uint8_t m_buffer[width * height / 8];
};
