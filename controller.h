#pragma once

/**
 * Controller interface.
 */
class Controller {
public:
    /**
     * Set target temperature.
     *
     * Set the temperature the controller should reach.
     *
     * @param temperature Target temperature in degree Celsius.
     */
    virtual void set_temperature(float temperature) = 0;

    /**
     * Get current temperature.
     *
     * @return Current temperature.
     */
    virtual float temperature() = 0;

    /**
     * Turn stirrer on or off.
     *
     * @param on @c true if stirrer is to be turned on, @c false if not.
     */
    virtual void set_stirrer_on(bool on) = 0;

    /**
     * Return whether stirrer is on or not.
     *
     * @return @c true if stirrer is on else @c false.
     */
    virtual bool stirrer_is_on() = 0;

    /**
     * Turn heater on or off.
     *
     * @param on @c true if heater is to be turned on, @c false if not.
     */
    virtual void set_heater_on(bool on) = 0;

    /**
     * Retrieve location of variable holding if heater is on or off.
     *
     * @return @c true if heater is on else @c false.
     */
    virtual bool heater_is_on() = 0;
};

/**
 * A software mock controller that does not access any real hardware.
 *
 * All values are set and returned immediately, i.e there is simulation of a
 * heating curve.
 */
class MockController : public Controller {
    void set_temperature(float temperature) final;

    float temperature() final;

    void set_stirrer_on(bool is_on) final;

    bool stirrer_is_on() final;

    void set_heater_on(bool is_on) final;

    bool heater_is_on() final;

private:
    float m_temperature{20.0f};
    bool m_stirrer_on{false};
    bool m_heater_on{false};
};
