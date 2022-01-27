#include <SPI.h>
#include "ssd1327.h"

Ssd1327::Ssd1327(byte rst, byte dc, byte din, byte clk)
: m_rst{rst}
, m_dc{dc}
, m_din{din}
, m_clk{clk}
{}

void Ssd1327::command(uint8_t cmd)
{
    digitalWrite(m_dc, LOW);
    SPI.transfer(cmd);
}

void Ssd1327::begin()
{
    pinMode(m_rst, OUTPUT);
    pinMode(m_dc, OUTPUT);
    
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);

    SPI.begin();

    SPI.setClockDivider(SPI_CLOCK_DIV2);

    // reset display
    digitalWrite(m_rst, HIGH);
    delay(10);
    digitalWrite(m_rst, LOW);
    delay(10);
    digitalWrite(m_rst, HIGH);

    // Set the initialization registers
    command(0xae);    // turn off oled panel, "sleep mode"

    command(0x15);    //set column address
    command(0x00);    //start column   0
    command(0x7f);    //end column   127
    // command(0x3f);    //end column   63

    command(0x75);    //set row address
    command(0x00);    //start row   0
    // command(0x7f);    //end row   127
    command(0x3f);    // end row 63

    command(0x81);    //set contrast control
    command(0x80);    // contrast value (double byte 1 to 256, 0x80=128)

    command(0xa0);    //gment remap (see OLED_ScanDir or OLED_SetGramScanWay)
    // command(0x51);    //51 --> b01010001 --> Enable COM-split // enable COM re-map // enable horizontal address increment // disable nibble-remap // enable column address remap
    //command(0x50);    //50 --> b01010000 --> Enable COM-split // enable COM re-map // enable horizontal address increment // disable nibble-remap // disable column address remap
    command(0x53);      //53 --> b01010011 --> Enable COM-split // enable COM re-map // enable horizontal address increment // enable nibble-remap // enable column address remap
    // command(0x55);    //50 --> b01010100 --> Enable COM-split // enable COM re-map // enable horizontal address increment // disable nibble-remap // enable column address remap

    command(0xa1);    //start line
    command(0x00);    // value (0 to 127)

    command(0xa2);    //display offset
    command(0x00);    // value (0 to 127)

    command(0xa4);    // normal display
    
    command(0xa8);    //set multiplex ratio "MUX ratio)
    command(0x7f);    // 0x7f = 127 (128MUX RESET)

    command(0xb1);    //set phase leghth
    command(0xf1);

    command(0xb3);    //set dclk
    command(0x00);    //80Hz:0xc1 90Hz:0xe1   100Hz:0x00   110Hz:0x30 120Hz:0x50   130Hz:0x70     01

    command(0xab);    // set Voltage regulator
    command(0x01);    // 0x01 = internal voltage regulator

    command(0xb6);    //set phase leghth (second pre charge period)
    command(0x0f);

    command(0xbe);    // select COM deselect voltage
    command(0x0f);    // value should only have three bytes -> 0x0f equals 0x07 --> 0.86*Vcc ?

    command(0xbc);    // set pre-charge voltage
    command(0x08);    // 0x08 = VCCOM

    command(0xd5);    // Function Selection B
    command(0x62);    // 0x62 = 0b01100010

    command(0xfd);    // Select command lock
    command(0x12);    // 0x12 = 0b00010010 (unlocked)

    command(0xAF);    //Turn on the OLED display

    // erase lower half (unused) part of display
    clear();          // set buffer (half display size) to all zero
    command(0x75);    //set row address for lower half
    command(0x40);    //start row   64
    command(0x7f);    //end row   127
    flush();          // write buffer to selected row range
    command(0x75);    //set row address back to upper half
    command(0x0);     //start row   64
    command(0x3f);    //end row   127
}

void Ssd1327::clear()
{
    for (size_t i = 0; i < width * height / 8; i++) {
        m_buffer[i] = 0;
    }
}

void Ssd1327::flush()
{
    uint8_t *buffer = m_buffer;

    // write data
    digitalWrite(m_dc, HIGH);
    for (uint8_t page = 0; page < 8; page++) {
        for (size_t i = 0; i < width; i++) {
            // change to 4bit grayscale
            // each bit should be replaced by b0000 or b1111 depending on its value
            for (uint8_t j = 0; j < 4; j++) {    // iterate over each 2bit pair in single byte of buffer
                uint8_t temp_buffer = buffer[i] >> (j*2);
                // 1. mask to lower two bits
                temp_buffer &= 0b00000011;
                // 2. replace 2bits by grayscale byte
                switch(temp_buffer) {
                    case 0b00:
                        SPI.transfer(0b00000000);
                        break;
                    case 0b01:
                        SPI.transfer(0b00001111);
                        break;
                    case 0b10:
                        SPI.transfer(0b11110000);
                        break;
                    case 0b11:
                        SPI.transfer(0b11111111);
                        break;
                };
            }
        }
        buffer += width;
    }
}

void Ssd1327::draw_pixel(uint8_t x, uint8_t y)
{
    if (x > width || y > height)
        return;

    m_buffer[(x / 8) + y * (width / 8)] |= 1 << (x % 8);    // example: first byte 0xFF equals horizontal line starting upper left and 8px length --> 9px length woud imply added second byte = b1 (LSBF)
}

void Ssd1327::draw_bitmap(uint8_t x, uint8_t y, Bitmap&& bitmap)
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
