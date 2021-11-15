#pragma once

#include <Arduino.h>

struct Bitmap {
    const uint8_t width;
    const uint8_t height;
    const uint8_t *data;
};

class Sh1106 {
public:
    Sh1106(byte rst=9, byte dc=8, byte cs=10, byte din=11, byte clk=13);

    void begin();
    void clear();
    void flush();
    void draw_pixel(uint8_t x, uint8_t y);
    void draw_bitmap(uint8_t x, uint8_t y, Bitmap&& bitmap);

private:
    void command(uint8_t cmd);

    const byte m_rst;
    const byte m_dc;
    const byte m_cs;
    const byte m_din;
    const byte m_clk;
    static const size_t m_width{128};
    static const size_t m_height{64};
    uint8_t m_buffer[m_width * m_height / 8];
};
