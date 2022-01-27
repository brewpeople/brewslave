#include <SPI.h>
#include "sh1107.h"

Sh1107::Sh1107(byte rst, byte dc, byte din, byte clk)
: m_rst{rst}
, m_dc{dc}
, m_din{din}
, m_clk{clk}
{}

void Sh1107::command(uint8_t cmd)
{
    digitalWrite(m_dc, LOW);
    SPI.transfer(cmd);
}

void Sh1107::begin()
{
    pinMode(m_rst, OUTPUT);
    pinMode(m_dc, OUTPUT);
    SPI.begin();

    SPI.setClockDivider(SPI_CLOCK_DIV8);

    digitalWrite(m_rst, HIGH);
    delay(10);
    digitalWrite(m_rst, LOW);
    delay(10);
    digitalWrite(m_rst, HIGH);

    // numbers indicate relevant section in datasheet

    command(0xAE); //--11. turn off oled panel

    // === commands relevant to orientation ===
    
    // column address is set prior sending data anyway, so this here is not necessary
    command(0x00); //--1. Set low column address 0x00~0x0F (address % 16)
    command(0x12); //--2. Set high column address 0x10~0x17 (0x10 + floor(address / 16))

    command(0x20); //--3. Set Page Addressing Mode

    command(0xA0); //--5. Set SEG/Column Mapping

    command(0xD3); //--9. Set display offset (two byte command)
    command(0x00); //--   value display start line COM0-127 -> 0x00~0x7F

    command(0xC8); //--13. Set COM/Row Scan Direction 0xC0/0xC8 (flips in short side direction)


    // === commands not relevant to orientation but non-default values ===
    command(0x81); //--4. Set contrast control register (double byte command)
    command(0xFF); //--contrast 0x00~0xFF

    command(0xD9); //--15. Set pre-charge period (two byte command)
    command(0xF1); //--value, Set Pre-Charge as 15 Clocks & Discharge as 1 Clock

    command(0xDB); //--16. Set vcomh (two byte command)
    command(0x40); //--Set VCOM Deselect Level


    // === commands with default values ===
    command(0xA8); //--6. Set multiplex ratio(1 to 64, double byte command)
    command(0x7F); //--0x00~0x07F (0x7F: default)

    command(0xA4); //--7. Disable 'Entire Display On' (0xa4:default / 0xa5: all on)

    command(0xA6); //--8. Set normal display (0xA6: normal, 0xA7: inverted)

    command(0xAD); //--10. Set DC-DC Settings (two byte command)
    command(0x81); //--default

    command(0xd5); //--14. Set display clock divide ratio/oscillator frequency (two byte command)
    command(0x80); //--value(0x00~0xFF), 0x80: default divider

    // clear_ram(); // for debug/orientation changes, keep for now

    command(0xAF); //--11. turn on oled panel (after all settings were completed)
}

void Sh1107::clear()
{
    for (size_t i = 0; i < width * height / 8; i++) {
        m_buffer[i] = 0;
    }
}

/**
 * clears entire controller RAM since it is 128x128 while display is 128x64
 * keep this for now for debug purposes
 */
// void Sh1107::clear_ram()
// {
    // for (uint8_t page = 0; page < 16; page++) {
        // // set page address
        // command(0xB0 + page);
        // // set low column address (address % 16)
        // command(0x00);
        // // set high column address (0x10 + floor(address / 16))
        // command(0x10);
        // // write data
        // digitalWrite(m_dc, HIGH);

        // for (size_t i = 0; i < 128; i++) {
            // SPI.transfer(0);
        // }
    // }
// }

void Sh1107::flush()
{
    uint8_t *buffer = m_buffer;

    for (uint8_t page = 0; page < 16; page++) {
        // set page address
        command(0xB0 + page);
        // set low column address (address % 16)
        command(0x00);
        // set high column address (0x10 + floor(address / 16))
        command(0x12);
        // write data
        digitalWrite(m_dc, HIGH);

        for (size_t i = 0; i < height; i++) {
            SPI.transfer(buffer[i]);
        }

        buffer += height;
    }
}

void Sh1107::draw_pixel(uint8_t x, uint8_t y)
{
    if (x > width || y > height)
        return;

    m_buffer[(x / 8) * height + y] |= 1 << (x % 8);
}

void Sh1107::draw_bitmap(uint8_t x, uint8_t y, Bitmap&& bitmap)
{
    uint8_t byte_width = (bitmap.width + 7) / 8;

    for (uint8_t j = 0; j < bitmap.height; j++){
        for (uint8_t i = 0; i < bitmap.width; i ++){
            // Seems stupid to read the same byte over and over again ...
            if (pgm_read_byte(bitmap.data + j * byte_width + i / 8) & (128 >> (i & 7))) {
                draw_pixel(x+i, y+j);
            }
        }
    }
}
