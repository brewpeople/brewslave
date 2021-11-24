#pragma once

/**
 * Abstract button encoder interface, i.e. a combination of rotary encoder with
 * builtin button.
 */
class ButtonEncoder {
public:
    enum class Direction {
        None,
        Clockwise,
        CounterClockwise,
    };

    /**
     * Update encoder state, ideally in an interrupt.
     */
    virtual void update() = 0;

    /**
     * Get current direction.
     */
    virtual Direction direction() = 0;

    /**
     * Return @c true if button was pressed.
     */
    virtual bool pressed() = 0;
};

class MockEncoder : public ButtonEncoder {
public:
    void update() final {}
    Direction direction() final { return Direction::None; }
    bool pressed() final { return false; }
};
