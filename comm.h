#pragma once

#include <Arduino.h>

class Controller;

/**
 * Brewslave communication protocol parser/handler.
 *
 * It takes a controller used to set the target temperatures, read the current
 * temperatures, and read the state of heaters.
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
