#pragma once

#include <Arduino.h>

class Controller;

/**
 * Brewslave communication protocol parser/handler.
 *
 * It takes a controller used to set the target temperature, read the current
 * temperature, and read the state of heater.
 *
 * TODO: we could split the controller interface into one that the comm object
 * uses and one that allows more mutability.
 */
class Comm {
public:
    Comm(Controller& control);

    /**
     * Call on serialEvent() to trigger serial processing.
     */
    void process_serial_data();

private:
    Controller& m_controller;
};
