#pragma once

#include "display.h"
#include <Arduino.h>

class Sh1107 : public Display {
public:
    Sh1107(byte rst = 12, byte dc = 10, byte din = 11, byte clk = 13);

    void begin() final;
    void clear() final;
    void flush() final;
    void draw_pixel(uint8_t x, uint8_t y) final;
    void draw_bitmap(uint8_t x, uint8_t y, Bitmap&& bitmap) final;

private:
    void command(uint8_t cmd);
    void clear_ram();

    const byte m_rst;
    const byte m_dc;
    const byte m_din;
    const byte m_clk;
    uint8_t m_buffer[width * height / 8];
};
