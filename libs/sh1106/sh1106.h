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

    static constexpr size_t width{128};
    static constexpr size_t height{64};

private:
    void command(uint8_t cmd);

    const byte m_rst;
    const byte m_dc;
    const byte m_cs;
    const byte m_din;
    const byte m_clk;
    uint8_t m_buffer[width * height / 8];
};
