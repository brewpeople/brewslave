#ifdef HAVE_CONFIG_H
    #include "config.h"
#else
    /* In the Arduino IDE, we enable the Nokia display by default. */
    #define WITH_LCD5110    1
    #define WITH_DS18B20    1
    #define WITH_KALMAN     1
#endif

#ifdef WITH_KALMAN
    #include <MatrixMath.h>
    #include <TempKalman.h>
#endif

#include "brew_control_protocol.h"
#include <crc.h>

namespace bm = brewmeister;


/* PIN DEFINITIONS */

#define RELAY_GFA           12  // GFA relay pin
#define RELAY_MOTOR         11  // motor relay pin

#ifdef WITH_DS18B20
    #include <OneWire.h>
    #include <DallasTemperature.h>

    #define ONE_WIRE_BUS    2   // DS18B20 sensor data pin
#endif

#ifdef WITH_LCD5110
    #include <LCD5110_Basic.h>

    #define LCD_RST         6
    #define LCD_CE          7
    #define LCD_DC          5
    #define LCD_DIN         4
    #define LCD_CLK         3
#endif

#ifdef WITH_NTC
    #include "ntc.h"
    #define NTC_PIN         0
#endif


/* CONSTANTS */

#define RELAY_ON            0
#define RELAY_OFF           1

#define STATE_MANUAL            0x00
#define STATE_HEAT_CONTROL      0x01
#define STATE_COOLING_CONTROL   0x02

#define DELTA_FIRST_LIMIT       5       // lower offset to set temperature [C] at which overshooting is considered (> DELTA_TEMP_FIRST)
#define DELTA_TEMP_FIRST        1.5     // lower offset to set temperature [C] at which heating stops to avoid overshooting (> DELTA_TEMP_LOW)
#define DELTA_TIME_FIRST        60      // break time [s] to avoid overshooting
#define DELTA_TEMP_HIGH         0.5     // upper limit temperature offset [C] for two-level-controller
#define DELTA_TEMP_LOW          0.5     // lower limit temperature offset [C] for two-level-controller
#define DELTA_TIME              60      // delay time between two-level-controller switches


/* SETTINGS */

#define TEMP_RESOLUTION         12
#define TEMP_EPSILON_ERROR      0.0001
#define ONEWIRE_CRC             1
#define SERIAL_BUFFER_SIZE      7       // define the size (amount of bytes) of the serial buffer


/* INITIALIZE INSTANCES */

#ifdef WITH_DS18B20
    OneWire ourWire(ONE_WIRE_BUS);
    DallasTemperature sensors(&ourWire);
#endif

#ifdef WITH_LCD5110
    LCD5110 myGLCD(LCD_CLK, LCD_DIN, LCD_DC, LCD_RST, LCD_CE);
#endif

#ifdef WITH_NTC
    NTC ntc(NTC_PIN);
#endif

#ifdef WITH_KALMAN
    TempKalmanFilter temp_flt(20.0, 1.0);
#endif


/* GLOBAL VARIABLES */

extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

extern uint8_t img_line[];
extern uint8_t test_position[];
extern uint8_t img_motor_off[];
extern uint8_t img_motor_on[];
extern uint8_t img_gfa_off[];
extern uint8_t img_gfa_on[];

byte slaveState = STATE_MANUAL;         // stores the current brewslave state


byte tempSensorAddr[8];                 // cache for temperature sensor address
boolean tempSensorStatus = false;

float temperature = -999.1337;
float temp_set = 42.4242;

byte serialBuffer[SERIAL_BUFFER_SIZE];

boolean overshooting = false;
long timerGFA = -1000 * DELTA_TIME;


/* TEST VARIABLES */

float start = 0;
float ende  = 0;


/* FORWARD DECLARATIONS */

#ifdef WITH_DS18B20
void resetTempSensor();
#endif

#ifdef WITH_LCD5110
void displayRefresh();
#endif

boolean getGFA();
boolean getMotor();
void setGFA(boolean on);
void setMotor(boolean on);
void twoLevelHeatController();


void setup()
{
    noInterrupts();                                         // disable all interrupts

    /* disable relays at startup */

    digitalWrite(RELAY_GFA, RELAY_OFF);
    digitalWrite(RELAY_MOTOR, RELAY_OFF);
    pinMode(RELAY_GFA, OUTPUT);
    pinMode(RELAY_MOTOR, OUTPUT);

    #ifdef WITH_LCD5110
        myGLCD.InitLCD();
        myGLCD.setFont(SmallFont);
        myGLCD.print("Brewslave v0.3", CENTER, 0);
        myGLCD.print("Loading...", CENTER, 24);
    #endif

    #ifdef WITH_DS18B20
        sensors.begin();
        ourWire.reset_search();
        ourWire.search(tempSensorAddr);
        resetTempSensor();
    #endif

    noInterrupts();                                         // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;

    OCR1A = 62500;                                          // compare match register 16MHz/256/freq, 62500 = 1Hz
    TCCR1B |= (1 << WGM12);                                 // CTC mode
    TCCR1B |= (1 << CS12);                                  // 256 prescaler
    TIMSK1 |= (1 << OCIE1A);                                // enable timer compare interrupt
    interrupts();                                           // enable all interrupts

    // TBA: possibly increase baud rate
    delay(1000);
    Serial.begin(115200, SERIAL_8N1);

    #ifdef WITH_LCD5110
    for (int i=1; i<6; i++) {
        myGLCD.clrRow(i, 0, 83);
    }
    #endif
}

void loop()
{
    noInterrupts();

    #ifdef WITH_LCD5110
    displayRefresh();
    #endif

    interrupts();

    if (slaveState == STATE_HEAT_CONTROL) {
        twoLevelHeatController();
    }

    delay(1000);
}

#ifdef WITH_LCD5110
void displayRefresh() {
    uint8_t *image;

    myGLCD.clrRow(4,12,83);
    myGLCD.clrRow(5,12,83);

    // TBA: possibly add condition HIGH > 999.99
    if (tempSensorStatus) {
        if ((temperature < -99) || (temperature > 199)) {
            myGLCD.setFont(SmallFont);
            myGLCD.print("RANGE", 39, 40);
        } else {
            myGLCD.setFont(MediumNumbers);
            myGLCD.printNumF(temperature, 2, RIGHT, 32);
        }
    }
    else {
        myGLCD.setFont(SmallFont);
        myGLCD.print("ERROR", 33, 40);
    }

    image = getMotor() ? img_motor_on : img_motor_off;
    myGLCD.drawBitmap(0, 8, image, 42, 24);

    image = getGFA() ? img_gfa_on : img_gfa_off;
    myGLCD.drawBitmap(42, 8, image, 42, 24);

    if (slaveState == STATE_HEAT_CONTROL) {
        myGLCD.setFont(MediumNumbers);
        myGLCD.printNumF(temp_set, 0, LEFT, 40);
    }
    else {
        myGLCD.setFont(SmallFont);
        myGLCD.print("--", 0,40);
    }
}
#endif

ISR(TIMER1_COMPA_vect)                                      // timer compare interrupt service routine
{
    #ifdef WITH_DS18B20
    if (sensors.isConnected(tempSensorAddr) && tempSensorStatus) {
        temperature = sensors.getTempC(tempSensorAddr);     // get temperature from sensor
        if ((abs(temperature + 127.00) < TEMP_EPSILON_ERROR) || (abs(temperature - 0.00) < TEMP_EPSILON_ERROR) || (abs(temperature -85.00) < TEMP_EPSILON_ERROR)) {
            tempSensorStatus = false;
        }

        // request new temperature conversion
        sensors.requestTemperatures();
    }
    else {
        tempSensorStatus = false;
        ourWire.reset_search();
        ourWire.search(tempSensorAddr);
        resetTempSensor();
    }
    #elif WITH_NTC
    temperature = ntc.temperature();
    #endif
}


#ifdef WITH_DS18B20
/* reconfigures temperature sensor, neccessary after lost connection */
void resetTempSensor() {
    if ((tempSensorAddr[0] == 0x28) && sensors.validAddress(tempSensorAddr) && sensors.isConnected(tempSensorAddr)) {
        tempSensorStatus = true;
        sensors.setResolution(TEMP_RESOLUTION);
        sensors.setWaitForConversion(true);                 // library will delay according to DS18B20 datasheet i.e. 750ms at 12bit accuracy
        sensors.requestTemperatures();
        temperature = sensors.getTempC(tempSensorAddr);
        sensors.setWaitForConversion(false);
    }
    else {
        tempSensorStatus = false;
    }
}
#endif

// TBA possible crc8 verification
void processSerialCommand() {
    switch (serialBuffer[0]) {
        case bm::READ:
            switch (serialBuffer[1]) {               // GET request by master
                case bm::TEMP:
                    Serial.write((byte*) &temperature, sizeof(float));
                    break;
                case bm::HEAT:
                    Serial.write(getGFA());
                    break;
                case bm::STIR:
                    Serial.write(getMotor());
                    break;
                case 0xF4:
                    Serial.write(tempSensorStatus);
                    break;
                default:
                    break;
            }
            break;
        case bm::WRITE:
            switch (serialBuffer[1]) {               // SET request by master
                case bm::TEMP:
                    temp_set = *((float *) &(serialBuffer[2]));
                    break;
                case bm::HEAT:
                    setGFA(serialBuffer[2]);
                    break;
                case bm::STIR:
                    setMotor(serialBuffer[2]);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

/* serialEvent() is an interrupt routine called when incoming serial data ist available */

void serialEvent() {
    int count = 0;

    while ((Serial.available() > 0) && (count < SERIAL_BUFFER_SIZE)) {
        serialBuffer[count] = Serial.read();
        count++;
    }
    
    if ((count == SERIAL_BUFFER_SIZE) && (crcSlow(serialBuffer, sizeof(serialBuffer)) == 0)) {
        Serial.write(serialBuffer[SERIAL_BUFFER_SIZE-1]);
        processSerialCommand();
    }
}

boolean getGFA() {
    return digitalRead(RELAY_GFA) != RELAY_OFF;
}

boolean getMotor() {
    return digitalRead(RELAY_MOTOR) != RELAY_OFF;
}

void setGFA(boolean on) {
    digitalWrite(RELAY_GFA, on ? RELAY_ON : RELAY_OFF);
}

void setMotor(boolean on) {
    digitalWrite(RELAY_MOTOR, on ? RELAY_ON : RELAY_OFF);
}

void twoLevelHeatController() {
    boolean gfa_delta_reached = millis() >= timerGFA + DELTA_TIME * 1000;

    // if temperature is more than DELTA_FIRST_LIMIT below set temperature, overshooting protection will be enabled
    if (temperature <= (temp_set-DELTA_FIRST_LIMIT)) {
        setGFA(true);
        overshooting = true;
    }

    // if overshooting protection is enabled and temperature is less than DELTA_TEMP_FIRST below set temperature, heating stops
    if (getGFA() && overshooting && (temperature >= (temp_set-DELTA_TEMP_FIRST))) {
        setGFA(false);
        timerGFA = millis();
    }

    // continues heating after DELTA_TIME_FIRST seconds for overshooting protection
    if (!getGFA() && overshooting && (millis() >= timerGFA + DELTA_TIME_FIRST * 1000)) {
        setGFA(true);
        overshooting = false;
        timerGFA = millis();
    }

    // two-level-controller lower limit
    if (!getGFA() && !overshooting && (temperature <= (temp_set-DELTA_TEMP_LOW)) && gfa_delta_reached) {
        setGFA(true);
        overshooting = false;
        timerGFA = millis();
    }

    // two-level-controller upper limit
    if (getGFA() && (temperature >= (temp_set+DELTA_TEMP_HIGH)) && gfa_delta_reached) {
        setGFA(false);
        overshooting = false;
        timerGFA = millis();
    }
}
