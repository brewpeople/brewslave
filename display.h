#pragma once

#include <Arduino.h>

/**
 * A bitmap description.
 */
struct Bitmap {
    /// Width of bitmap in number of pixels.
    const uint8_t width;
    /// Height of bitmap in number of pixels.
    const uint8_t height;
    /// Pointer to bitmap data with at least (width * height / 8) bytes.
    const uint8_t* data;
};

class Display {
public:
    /**
     * One-time initialization to be called in setup().
     */
    virtual void begin() = 0;

    /**
     * Clear the frame buffer.
     */
    virtual void clear() = 0;

    /**
     * Write the frame buffer to the display.
     */
    virtual void flush() = 0;

    /**
     * Draw a pixel at coordinate (@p x, @p y) if it falls within width and
     * height.
     *
     * @param x X coordinate.
     * @param y Y coordinate.
     */
    virtual void draw_pixel(uint8_t x, uint8_t y) = 0;

    /**
     * Draw a bitmap beginning with the top-left corner at (@p x, @p y).
     *
     * @param x X corner of the bitmap draw position.
     * @param y Y corner of the bitmap draw position.
     * @param bitmap Bitmap description.
     */
    virtual void draw_bitmap(uint8_t x, uint8_t y, Bitmap&& bitmap) = 0;

    // This is slightly unfortunate and fixes all deriving displays to be of
    // this dimension.
    static constexpr size_t width{128};
    static constexpr size_t height{64};
};

/**
 * A non-functional display.
 */
class MockDisplay : public Display {
public:
    void begin() final {}

    void clear() final {}

    void flush() final {}

    void draw_pixel(uint8_t, uint8_t) final {}

    void draw_bitmap(uint8_t, uint8_t, Bitmap&&) final {}
};
