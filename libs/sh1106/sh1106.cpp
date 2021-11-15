#include <SPI.h>
#include "sh1106.h"

Sh1106::Sh1106(byte rst, byte dc, byte cs, byte din, byte clk)
: m_rst{rst}
, m_dc{dc}
, m_cs{cs}
, m_din{din}
, m_clk{clk}
{}

void Sh1106::command(uint8_t cmd)
{
    digitalWrite(m_dc, LOW);
    SPI.transfer(cmd);
}

void Sh1106::begin()
{
    pinMode(m_rst, OUTPUT);
    pinMode(m_dc, OUTPUT);
    pinMode(m_cs, OUTPUT);
    SPI.begin();

    SPI.setClockDivider(SPI_CLOCK_DIV128);

    digitalWrite(m_cs, LOW);
    digitalWrite(m_rst, HIGH);
    delay(10);
    digitalWrite(m_rst, LOW);
    delay(10);
    digitalWrite(m_rst, HIGH);

    command(0xAE); //--turn off oled panel
    command(0x02); //---set low column address
    command(0x10); //---set high column address
    command(0x40); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    command(0x81); //--set contrast control register
    command(0xA0); //--Set SEG/Column Mapping
    command(0xC0); //Set COM/Row Scan Direction
    command(0xA6); //--set normal display
    command(0xA8); //--set multiplex ratio(1 to 64)
    command(0x3F); //--1/64 duty
    command(0xD3); //-set display offset    Shift Mapping RAM Counter (0x00~0x3F)
    command(0x00); //-not offset
    command(0xd5); //--set display clock divide ratio/oscillator frequency
    command(0x80); //--set divide ratio, Set Clock as 100 Frames/Sec
    command(0xD9); //--set pre-charge period
    command(0xF1);  //Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    command(0xDA); //--set com pins hardware configuration
    command(0x12);
    command(0xDB); //--set vcomh
    command(0x40); //Set VCOM Deselect Level
    command(0x20); //-Set Page Addressing Mode (0x00/0x01/0x02)
    command(0x02); //
    command(0xA4); // Disable Entire Display On (0xa4/0xa5)
    command(0xA6); // Disable Inverse Display On (0xa6/a7)
    command(0xAF); //--turn on oled panel
}

void Sh1106::clear()
{
    for (size_t i = 0; i < m_width * m_height / 8; i++) {
        m_buffer[i] = 0;
    }
}

void Sh1106::flush()
{
    uint8_t *buffer = m_buffer;

    for (uint8_t page = 0; page < 8; page++) {
        // set page address
        command(0xB0 + page);
        // set low column address
        command(0x02);
        // set high column address
        command(0x10);
        // write data
        digitalWrite(m_dc, HIGH);

        for (size_t i = 0; i < m_width; i++) {
            SPI.transfer(buffer[i]);
        }

        buffer += m_width;
    }
}

void Sh1106::draw_pixel(uint8_t x, uint8_t y)
{
    if (x > m_width || y > m_height)
        return;

    m_buffer[x + (y / 8) * m_width] |= 1 << (y % 8);
}

void Sh1106::draw_bitmap(uint8_t x, uint8_t y, Bitmap&& bitmap)
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
