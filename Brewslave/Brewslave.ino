#ifdef HAVE_CONFIG_H
    #include "config.h"
#else
    /* In the Arduino IDE, we enable the Nokia display by default. */
    #define WITH_LCD5110    1
    #define WITH_DS18B20    1
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
    #define ONEWIRE_SEARCH 1
    #define ONEWIRE_CRC 1
    #define ONEWIRE_CRC8_TABLE 0
    #define ONEWIRE_CRC16 0

    #include <OneWire.h>
    #include <DallasTemperature.h>

    #ifdef WITH_BUTTONS
        #define ONE_WIRE_BUS    9   // DS18B20 sensor data pin
    #else
        #define ONE_WIRE_BUS    2   // DS18B20 sensor data pin
    #endif
#endif

#ifdef WITH_LCD5110
    #include <LCD5110_Basic.h>

    #define LCD_RST         6
    #define LCD_CE          7
    #define LCD_DC          5
    #define LCD_DIN         4
    #ifdef WITH_BUTTONS
        #define LCD_CLK         8
    #else
        #define LCD_CLK         3
    #endif
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
#define SERIAL_BUFFER_SIZE      7       // define the size (amount of bytes) of the serial buffer
#define TEMP_SENSOR_TIMEOUT     60      // seconds before heat control stops automatically


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
#ifdef DEBUG_SERIAL
extern uint8_t box16[];
extern uint8_t box16_up[];
extern uint8_t box16_down[];
extern uint8_t box16_cross[];
extern uint8_t box16_heat[];
extern uint8_t box16_heat_inv[];
extern uint8_t box16_stir[];
extern uint8_t box16_stir_inv[];
#else
#ifdef DEBUG_STATES
extern uint8_t box16_heat[];
extern uint8_t box16_heat_inv[];
extern uint8_t box16_stir[];
extern uint8_t box16_stir_inv[];
#else
extern uint8_t img_motor_off[];
extern uint8_t img_motor_on[];
extern uint8_t img_gfa_off[];
extern uint8_t img_gfa_on[];
#endif
#endif

byte slaveState = STATE_MANUAL;   // stores the current brewslave state

byte tempSensorAddr[8];                 // cache for temperature sensor address
boolean tempSensorStatus = false;
boolean crc8Correct = true;

float temperature = -999.1337;
float temp_set = 35;

byte serialBuffer[SERIAL_BUFFER_SIZE];

#ifdef WITH_BUTTONS
unsigned long timerSwitchMotor = 0;
unsigned long timerSwitchGFA = 0;
unsigned long timerDebounce = 1000;
#endif

boolean overshooting = false;
unsigned long timerGFA = 0;

unsigned long timerTempSensorLastSeen = 0;

#ifdef DEBUG_SERIAL
#define COMMAND_SET     1
#define COMMAND_GET     2
#define COMMAND_ERROR   3

byte lastCommandState = 0;

int com_error = 0;
int com_set = 0;
int com_get = 0;
#else
#ifdef DEBUG_STATES
#define DEBUG_STATES_STRING_LENGTH 3
//char debug_string1[DEBUG_STATES_STRING_LENGTH] = "001";
char *debug_strings[] = {"   ", "   ", "   ", "   ", "   ", "   ", "   ", "   ", "   "};


#endif
#endif

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
#ifdef WITH_BUTTONS
void switchMotor();
void switchGFA();
#endif
void twoLevelHeatController();
void debug_state_add(char* msg);

/* SETUP FUNCTION */

void setup()
{
    noInterrupts();                     // disable all interrupts

    /* disable relays at startup */

    digitalWrite(RELAY_GFA, RELAY_OFF);
    digitalWrite(RELAY_MOTOR, RELAY_OFF);
    pinMode(RELAY_GFA, OUTPUT);
    pinMode(RELAY_MOTOR, OUTPUT);

    #ifdef WITH_LCD5110
        myGLCD.InitLCD();
        myGLCD.setFont(SmallFont);
        myGLCD.print((char*)"Brewslave v0.3", CENTER, 0);
        myGLCD.print((char*)"Loading...", CENTER, 24);
        #ifdef DEBUG_SERIAL
        myGLCD.print((char*)"Debug Serial", CENTER, 32);
        #else
        #ifdef DEBUG_STATES
        myGLCD.print((char*)"Debug States", CENTER, 32);
        debug_state_add("Ini");
        #endif
        #endif
        #ifdef DEBUG_DS18B20
        myGLCD.print((char*)"Debug DS18B20", CENTER, 40);
        #endif
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
    
    #ifdef WITH_BUTTONS
        attachInterrupt(0, switchMotor, HIGH);                // pin interrupt for manual override button Motor
        attachInterrupt(1, switchGFA, RISING);                  // pin interrupt for manual override button Motor
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
    #ifdef DEBUG_DS18B20
    myGLCD.setFont(SmallFont);
    if (tempSensorStatus) {
        myGLCD.print((char*)"OK",18,32);
    } else {
        myGLCD.print((char*)"ERR",18,32);
    }
    myGLCD.printNumF(temperature, 3, RIGHT, 32);
    #else
    if (tempSensorStatus) {
        if ((temperature < -99) || (temperature > 199)) {
            myGLCD.setFont(SmallFont);
            myGLCD.print((char*)"RANGE", 39, 40);
        } else {
            myGLCD.setFont(MediumNumbers);
            myGLCD.printNumF(temperature, 2, RIGHT, 32);
        }
    }
    else {
        myGLCD.setFont(SmallFont);
        myGLCD.print((char*)"ERROR", 33, 40);
    }
    #endif
    
    // display set-temperature
    
    if (slaveState == STATE_HEAT_CONTROL) {
        myGLCD.setFont(SmallFont);
        myGLCD.printNumI(temp_set, 0, 40);
    }
    else {
        myGLCD.setFont(SmallFont);
        myGLCD.print((char*)"--", 0,40);
    }
    
#ifdef DEBUG_SERIAL
    
    myGLCD.clrRow(0);
    myGLCD.clrRow(1);
    myGLCD.clrRow(2);
    myGLCD.clrRow(3);
    
    image = getMotor() ? box16_stir : box16_stir_inv;
    myGLCD.drawBitmap(0, 0, image, 16, 16);
    
    image = getGFA() ? box16_heat : box16_heat_inv;
    myGLCD.drawBitmap(0, 16, image, 16, 16);
    
    switch (lastCommandState) {
        case COMMAND_ERROR:
            image = box16_cross;
            break;
        case COMMAND_SET:
            image = box16_down;
            break;
        case COMMAND_GET:
            image = box16_up;
            break;
            
        default:
            image = box16;
            break;
    }
    myGLCD.drawBitmap(16, 16, image, 16, 16);
    lastCommandState = 0;
    
    
    // debug heat control
    /*
    if (slaveState == STATE_HEAT_CONTROL) {
        myGLCD.setFont(SmallFont);
        if (overshooting) {
            myGLCD.print("OS on",32,0);
        }
        myGLCD.print("TIME:",32,8);
        myGLCD.printNumI((millis()/1000),66,8);
        myGLCD.print("LAST:",32,16);
        myGLCD.printNumI((timerGFA/1000),66,16);
    }
    */
    
    // debug communication
    
    myGLCD.setFont(SmallFont);
    myGLCD.print((char*)"SERIAL",38,0);
    myGLCD.print((char*)"GET:",32,8);
    myGLCD.printNumI(com_get,60,8);
    myGLCD.print((char*)"SET:",32,16);
    myGLCD.printNumI(com_set,60,16);
    myGLCD.print((char*)"ERR:",32,24);
    myGLCD.printNumI(com_error,60,24);
    
    
#else
#ifdef DEBUG_STATES
    
    myGLCD.clrRow(0);
    myGLCD.clrRow(1);
    myGLCD.clrRow(2);
    myGLCD.clrRow(3);
    
    image = getMotor() ? box16_stir : box16_stir_inv;
    myGLCD.drawBitmap(0, 0, image, 16, 16);
    
    image = getGFA() ? box16_heat : box16_heat_inv;
    myGLCD.drawBitmap(0, 16, image, 16, 16);
    
    myGLCD.setFont(SmallFont);
    //myGLCD.invertFont(true);
    //myGLCD.print((char*)"ABC EFG IJK",18,0);
    myGLCD.invertText(true);
    myGLCD.print((char*)"   STATES  ",18,0);
    myGLCD.invertText(false);
    //myGLCD.invertFont(false);
    myGLCD.print(debug_strings[8],18,8);
    myGLCD.print(debug_strings[7],18,16);
    myGLCD.print(debug_strings[6],18,24);
    
    myGLCD.print(debug_strings[5],42,8);
    myGLCD.print(debug_strings[4],42,16);
    myGLCD.print(debug_strings[3],42,24);
    
    myGLCD.print(debug_strings[2],66,8);
    myGLCD.print(debug_strings[1],66,16);
    
    myGLCD.invertText(true);
    myGLCD.print(debug_strings[0],66,24);
    myGLCD.invertText(false);
  
#else
    
    image = getMotor() ? img_motor_on : img_motor_off;
    myGLCD.drawBitmap(0, 8, image, 42, 24);

    image = getGFA() ? img_gfa_on : img_gfa_off;
    myGLCD.drawBitmap(42, 8, image, 42, 24);

#endif
#endif
    
}
#endif

ISR(TIMER1_COMPA_vect)                                      // timer compare interrupt service routine
{
    #ifdef WITH_DS18B20
    if (sensors.isConnected(tempSensorAddr) && tempSensorStatus) {
        temperature = sensors.getTempC(tempSensorAddr);     // get temperature from sensor
        if ((abs(temperature + 127.00) < TEMP_EPSILON_ERROR) || (abs(temperature - 0.00) < TEMP_EPSILON_ERROR) || (abs(temperature - 85.00) < TEMP_EPSILON_ERROR)) {
            tempSensorStatus = false;
            #ifdef DEBUG_STATES
            debug_state_add("Trr");
            #endif
        } else {
            timerTempSensorLastSeen = millis();
            #ifdef DEBUG_STATES
            debug_state_add("Tok");
            #endif
        }

        // request new temperature conversion
        sensors.requestTemperatures();
    } else {
        tempSensorStatus = false;
        ourWire.reset_search();
        ourWire.search(tempSensorAddr);
        resetTempSensor();
        #ifdef DEBUG_STATES
        debug_state_add("Tnc");
        #endif
    }
    #elif WITH_NTC
    temperature = ntc.temperature();
    #endif
}

#ifdef DEBUG_STATES
/* adds new debug message and drops last message in stack */
void debug_state_add(char *msg) {
    int size = sizeof(debug_strings)/sizeof(char*);
    for (int i=size; i>=0; --i) {
        debug_strings[i] = debug_strings[i-1];
    }
    debug_strings[0] = msg;
}
#endif


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
        #ifdef DEBUG_STATES
        debug_state_add("Trc");
        #endif
    }
    else {
        tempSensorStatus = false;
        #ifdef DEBUG_STATES
        debug_state_add("Tcf");
        #endif
    }
}
#endif


void processSerialCommand() {
    byte commandBuffer[SERIAL_BUFFER_SIZE];
    byte* b = (byte*) &temperature;
    switch (serialBuffer[0]) {
        case bm::READ:
#ifdef DEBUG_SERIAL
            lastCommandState = COMMAND_GET;
            com_get++;
#endif
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
                case bm::HEAT_CONTROL:
                    commandBuffer[0] = bm::HEAT_CONTROL;
                    if (slaveState == STATE_HEAT_CONTROL) {
                        commandBuffer[1] = 1;
                    } else {
                        commandBuffer[1] = 0;
                    }
                    commandBuffer[6] = crcSlow(commandBuffer, SERIAL_BUFFER_SIZE-1);
                    Serial.write(commandBuffer, SERIAL_BUFFER_SIZE);
                    break;
                default:
                    break;
            }
            break;
        case bm::WRITE:
#ifdef DEBUG_SERIAL
            lastCommandState = COMMAND_SET;
            com_set++;
#endif
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
                case bm::HEAT_CONTROL:
                    slaveState = STATE_MANUAL;
                    setGFA(false);
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
#ifdef DEBUG_SERIAL
    else {
        lastCommandState = COMMAND_ERROR;
        com_error++;
    }
#endif
}

boolean getGFA() {
    return digitalRead(RELAY_GFA) ? RELAY_OFF : RELAY_ON;
}

boolean getMotor() {
    return digitalRead(RELAY_MOTOR) ? RELAY_OFF : RELAY_ON;
}

void setGFA(boolean on) {
    digitalWrite(RELAY_GFA, on ? RELAY_OFF : RELAY_ON);
//    if(getGFA()) {
//        setMotor(true);
//    }
}

void setMotor(boolean on) {
    digitalWrite(RELAY_MOTOR, on ? RELAY_OFF : RELAY_ON);
//    if(!getMotor()) {
//        setGFA(false);
//    }
}

void switchMotor() {
    if((millis() - timerSwitchMotor) > timerDebounce) {
        setMotor(!getMotor());
        timerSwitchMotor = millis();
#ifdef DEBUG_STATES
        if (getMotor() == RELAY_ON) {
            debug_state_add("BM1");
        } else {
            debug_state_add("BM0");
        }
#endif
    }
}

void switchGFA() {
    if((millis() - timerSwitchGFA) > timerDebounce) {
        setGFA(!getGFA());
        timerSwitchGFA = millis();
#ifdef DEBUG_STATES
        if (getGFA() == RELAY_ON) {
            debug_state_add("BH1");
        } else {
            debug_state_add("BH0");
        }
#endif
    }
}

void twoLevelHeatController() {
    #ifdef DEBUG_STATES
    debug_state_add("Con");
    #endif
    if (millis() >= (timerTempSensorLastSeen + TEMP_SENSOR_TIMEOUT * 1000)) {
        slaveState = STATE_MANUAL;
        setGFA(false);
        #ifdef DEBUG_STATES
        debug_state_add("tOT");
        #endif
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
