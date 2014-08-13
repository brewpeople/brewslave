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


/* TWO-LEVEL CONTROLLER */

#define DELTA_FIRST_LIMIT       5       // lower offset to set temperature [C] at which overshooting is considered (> DELTA_TEMP_FIRST)
#define DELTA_TEMP_FIRST        3       // lower offset to set temperature [C] at which heating stops to avoid overshooting (> DELTA_TEMP_LOW)
#define DELTA_TIME_FIRST        30      // break time [s] to avoid overshooting
#define DELTA_TEMP_HIGH         0.0     // upper limit temperature offset [C] for two-level-controller
#define DELTA_TEMP_LOW          0.2     // lower limit temperature offset [C] for two-level-controller
#define DELTA_TIME              30      // delay time between two-level-controller switches


/* SETTINGS */

#define TEMP_RESOLUTION         12
#define TEMP_EPSILON_ERROR      0.0001
#define ONEWIRE_CRC             1
#define SERIAL_BUFFER_SIZE      7       // define the size (amount of bytes) of the serial buffer
#define TEMP_SENSOR_TIMEOUT     10      // seconds before heat control stops automatically


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
#ifdef DEBUG_DISPLAY
extern uint8_t box16[];
#endif

byte slaveState = STATE_HEAT_CONTROL;         // stores the current brewslave state


byte tempSensorAddr[8];                 // cache for temperature sensor address
boolean tempSensorStatus = false;
boolean crc8Correct = true;

float temperature = -999.1337;
float temp_set = 35;

byte serialBuffer[SERIAL_BUFFER_SIZE];

boolean overshooting = false;
long timerGFA = -1000 * DELTA_TIME;

long timerTempSensorLastSeen = 0;

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

    delay(300);
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

    delay(800);
}

#ifdef WITH_LCD5110
void displayRefresh() {
    // myGLCD.InitLCD();                                    // avoid blank display
    
    uint8_t *image;

    myGLCD.clrRow(4,12,83);
    myGLCD.clrRow(5,12,83);
    
    // display temperature

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
    
    // display set-temperature
    
    if (slaveState == STATE_HEAT_CONTROL) {
        myGLCD.setFont(SmallFont);
        myGLCD.printNumI(temp_set, 0, 40);
    }
    else {
        myGLCD.setFont(SmallFont);
        myGLCD.print("--", 0,40);
    }
    
#ifdef DEBUG_DISPLAY
    
    myGLCD.clrRow(0);
    myGLCD.clrRow(1);
    
    myGLCD.drawBitmap(0,0, box16, 16, 16);
    myGLCD.drawBitmap(16,0, box16, 16, 16);
    myGLCD.drawBitmap(0,16, box16, 16, 16);
    myGLCD.drawBitmap(16,16, box16, 16, 16);
    
    if (!crc8Correct) {
        myGLCD.setFont(SmallFont);
        myGLCD.print("!",0, 16);
    }
    
    if (slaveState == STATE_HEAT_CONTROL) {
        myGLCD.setFont(SmallFont);
        if (overshooting) {
            myGLCD.print("O",8,0);
        }
        
        myGLCD.printNumI((millis()/1000),RIGHT,8);
        myGLCD.printNumI((timerGFA/1000),RIGHT,16);
    }
    
#else
    
    image = getMotor() ? img_motor_on : img_motor_off;
    myGLCD.drawBitmap(0, 8, image, 42, 24);

    image = getGFA() ? img_gfa_on : img_gfa_off;
    myGLCD.drawBitmap(42, 8, image, 42, 24);

#endif
    
}
#endif

ISR(TIMER1_COMPA_vect)                                      // timer compare interrupt service routine
{
    #ifdef WITH_DS18B20
    if (sensors.isConnected(tempSensorAddr) && tempSensorStatus) {
        temperature = sensors.getTempC(tempSensorAddr);     // get temperature from sensor
        if ((abs(temperature + 127.00) < TEMP_EPSILON_ERROR) || (abs(temperature - 0.00) < TEMP_EPSILON_ERROR) || (abs(temperature -85.00) < TEMP_EPSILON_ERROR)) {
            tempSensorStatus = false;
        } else {
            timerTempSensorLastSeen = millis();
        }

        // request new temperature conversion
        sensors.requestTemperatures();
    } else {
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
        timerTempSensorLastSeen = millis();
    }
    else {
        tempSensorStatus = false;
    }
}
#endif


void processSerialCommand() {
    byte commandBuffer[SERIAL_BUFFER_SIZE];
    byte* b = (byte*) &temperature;
    switch (serialBuffer[0]) {
        case bm::READ:
            switch (serialBuffer[1]) {               // GET request by master
                case bm::TEMP:
                    commandBuffer[0] = bm::TEMP;
                    commandBuffer[1] = (byte) tempSensorStatus;
                    commandBuffer[2] = b[0];
                    commandBuffer[3] = b[1];
                    commandBuffer[4] = b[2];
                    commandBuffer[5] = b[3];
                    commandBuffer[6] = crcSlow(commandBuffer, SERIAL_BUFFER_SIZE-1);
                    Serial.write(commandBuffer, SERIAL_BUFFER_SIZE);
                    break;
                case bm::HEAT:
                    commandBuffer[0] = bm::HEAT;
                    commandBuffer[1] = getGFA();
                    commandBuffer[6] = crcSlow(commandBuffer, SERIAL_BUFFER_SIZE-1);
                    Serial.write(commandBuffer, SERIAL_BUFFER_SIZE);
                    break;
                case bm::STIR:
                    commandBuffer[0] = bm::STIR;
                    commandBuffer[1] = getMotor();
                    commandBuffer[6] = crcSlow(commandBuffer, SERIAL_BUFFER_SIZE-1);
                    Serial.write(commandBuffer, SERIAL_BUFFER_SIZE);
                    break;
                default:
                    break;
            }
            break;
        case bm::WRITE:
            switch (serialBuffer[1]) {               // SET request by master
                case bm::TEMP:
                    temp_set = *((float *) &(serialBuffer[2]));
                    if (millis() < (timerTempSensorLastSeen + TEMP_SENSOR_TIMEOUT * 1000)) {
                        slaveState = STATE_HEAT_CONTROL;
                    }
                    Serial.write(serialBuffer[SERIAL_BUFFER_SIZE-1]);
                    break;
                case bm::HEAT:
                    setGFA(serialBuffer[2]);
                    Serial.write(serialBuffer[SERIAL_BUFFER_SIZE-1]);
                    break;
                case bm::STIR:
                    setMotor(serialBuffer[2]);
                    Serial.write(serialBuffer[SERIAL_BUFFER_SIZE-1]);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    for (int i=0; i < SERIAL_BUFFER_SIZE; i++) {
        serialBuffer[i] = 0;
    }
}

/* serialEvent() is an interrupt routine called when incoming serial data ist available */

void serialEvent() {
    int count = 0;

    while ((Serial.available() > 0) && (count < SERIAL_BUFFER_SIZE)) {
        serialBuffer[count] = Serial.read();
        count++;
    }
    
    crc8Correct = (crcSlow(serialBuffer, sizeof(serialBuffer)) == 0);
    if ((count == SERIAL_BUFFER_SIZE) && crc8Correct) {
        processSerialCommand();
    }
}

boolean getGFA() {
    return digitalRead(RELAY_GFA) ? RELAY_ON : RELAY_OFF;
}

boolean getMotor() {
    return digitalRead(RELAY_MOTOR) ? RELAY_ON : RELAY_OFF;
}

void setGFA(boolean on) {
    digitalWrite(RELAY_GFA, on ? RELAY_ON : RELAY_OFF);
}

void setMotor(boolean on) {
    digitalWrite(RELAY_MOTOR, on ? RELAY_ON : RELAY_OFF);
}

void twoLevelHeatController() {
    if (millis() >= (timerTempSensorLastSeen + TEMP_SENSOR_TIMEOUT * 1000)) {
        slaveState = STATE_MANUAL;
        setGFA(false);
        return;
    }
    
    boolean gfa_delta_reached = (millis() >= (timerGFA + (DELTA_TIME * 1000)));

    // if temperature is more than DELTA_FIRST_LIMIT below set temperature, overshooting protection will be enabled
    if (temperature <= (temp_set - DELTA_FIRST_LIMIT)) {
        setGFA(true);
        overshooting = true;
    }

    // if overshooting protection is enabled and temperature is less than DELTA_TEMP_FIRST below set temperature, heating stops
    if (getGFA() && overshooting && (temperature >= (temp_set - DELTA_TEMP_FIRST))) {
        setGFA(false);
        timerGFA = millis();
    }

    // continues heating after DELTA_TIME_FIRST seconds for overshooting protection
    if (!getGFA() && overshooting && (millis() >= (timerGFA + DELTA_TIME_FIRST * 1000))) {
        setGFA(true);
        overshooting = false;
        timerGFA = millis();
    }

    // two-level-controller lower limit
    if (!getGFA() && !overshooting && (temperature <= (temp_set - DELTA_TEMP_LOW)) && gfa_delta_reached) {
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
