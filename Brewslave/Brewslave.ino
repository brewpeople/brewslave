#ifdef HAVE_CONFIG_H
    #include "config.h"
#else
    /* In the Arduino IDE, we enable the Nokia display by default. */
    #define WITH_LCD5110    1
    #define WITH_DS18B20    1
#endif


/* PIN DEFINITIONS */

#define RELAY_GFA 12                                        // GFA relay pin
#define RELAY_MOTOR 11                                      // motor relay pin

#ifdef WITH_DS18B20
    #include <OneWire.h>
    #include <DallasTemperature.h>

    #define ONE_WIRE_BUS 2                                  // DS18B20 sensor data pin
#endif

#ifdef WITH_LCD5110
    #include <LCD5110_Basic.h>
    
    #define LCD_RST 6
    #define LCD_CE 7
    #define LCD_DC 5
    #define LCD_DIN 4
    #define LCD_CLK 3
#endif

#ifdef WITH_NTC
    #include "ntc.h"
#endif


/* CONSTANTS */

#define RELAY_ON 0
#define RELAY_OFF 1

extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

extern uint8_t img_line[];
extern uint8_t test_position[];
extern uint8_t img_motor_off[];
extern uint8_t img_motor_on[];
extern uint8_t img_gfa_off[];
extern uint8_t img_gfa_on[];

#define DELTA_FIRST_LIMIT 5                                 // lower offset to set temperature [C] at which overshooting is considered (> DELTA_TEMP_FIRST)
#define DELTA_TEMP_FIRST 1.5                                // lower offset to set temperature [C] at which heating stops to avoid overshooting (> DELTA_TEMP_LOW)
#define DELTA_TIME_FIRST 60                                 // break time [s] to avoid overshooting
#define DELTA_TEMP_HIGH 0.5                                 // upper limit temperature offset [C] for two-level-controller
#define DELTA_TEMP_LOW 0.5                                  // lower limit temperature offset [C] for two-level-controller
#define DELTA_TIME 60                                       // delay time between two-level-controller switches


/* SETTINGS */

#define TEMP_RESOLUTION 12
//#define ONEWIRE_CRC8_TABLE 1                                // lookuptable for crc8 check, fastert but more memory nessessary
#define SERIAL_BUFFER_SIZE 6                                // define the size (amount of bytes) of the serial buffer


/* INITIALIZE INSTANCES */

#ifdef WITH_DS18B20
    OneWire ourWire(ONE_WIRE_BUS);                          // initialize OneWire instance
    DallasTemperature sensors(&ourWire);                    // initialize DallasTemperature Library with OneWire instance
#endif

#ifdef WITH_LCD5110
    LCD5110 myGLCD(LCD_CLK, LCD_DIN, LCD_DC, LCD_RST, LCD_CE);  // initialize Nokia 5110 gLCD instance
#endif

#ifdef WITH_NTC
    NTC ntc(0);
#endif


/* GLOBAL VARIABLES */

byte tempSensorAddr[8];                                     // cache for temperature sensor address
boolean tempSensorStatus = false;

float temperature = -999.1337;                           
float temp_soll = 42.4242;

byte serialBuffer[SERIAL_BUFFER_SIZE];

boolean overshooting = false;
long timerGFA = -1000 * DELTA_TIME;

boolean twoLevelControllerStatus = false;

/* TEST VARIABLES */

float start = 0;
float ende  = 0;


/* FORWARD DECLARATIONS */

void resetTempSensor();
boolean getGFA();
boolean getMotor();
void setGFA(boolean on);
void setMotor(boolean on);
void displayRefresh();


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
        myGLCD.print("Brewmeister0.3", CENTER, 0);
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
    Serial.begin(9600);

    #ifdef WITH_LCD5110
    for(int i=1; i<6; i++) {
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
    
    delay(1000);
}

#ifdef WITH_LCD5110
void displayRefresh() {
    myGLCD.clrRow(4,12,83);
    myGLCD.clrRow(5,12,83);
    
    // TBA: possibly add condition HIGH > 999.99
    if(tempSensorStatus) {
        if(temperature < 0) {
            myGLCD.setFont(SmallFont);
            myGLCD.print("LOW", 39, 40);
        } else {
            myGLCD.setFont(MediumNumbers);
            myGLCD.printNumF(temperature, 2, RIGHT, 32);
        }
    } else {
        myGLCD.setFont(SmallFont);
        myGLCD.print("ERROR", 33, 40);
    }
    
    if(getMotor()) {
        myGLCD.drawBitmap(0, 8, img_motor_on, 42, 24);
    } else {
        myGLCD.drawBitmap(0, 8, img_motor_off, 42, 24);
    }
    
    if(getGFA()) {
        myGLCD.drawBitmap(42, 8, img_gfa_on, 42, 24);
    } else {
        myGLCD.drawBitmap(42, 8, img_gfa_off, 42, 24);
    }

    //myGLCD.drawBitmap(0, 8, test_position, 84, 24);
}
#endif

ISR(TIMER1_COMPA_vect)                                      // timer compare interrupt service routine
{
    //start = millis();
    #ifdef WITH_DS18B20
    if(sensors.isConnected(tempSensorAddr) && tempSensorStatus) {
        //Serial.print("T=");
        //Serial.println(temperature);
        temperature = sensors.getTempC(tempSensorAddr);     // get temperature from sensor
        if ((temperature == -127.00) || (temperature == 0.00) || (temperature == 85.00)) {
            tempSensorStatus = false;
        }
        sensors.requestTemperatures();                      // request new temperature conversion
    } else {
        tempSensorStatus = false;
        ourWire.reset_search();
        ourWire.search(tempSensorAddr);
        resetTempSensor();
    }
    #elif WITH_NTC
    temperature = ntc.temperature();
    #endif
  
    //ende = millis();
    //Serial.print(ende - start);
}


/* reconfigures temperature sensor, neccessary after lost connection */

void resetTempSensor() {
    #ifdef WITH_DS18B20
    if((tempSensorAddr[0] == 0x28) && sensors.validAddress(tempSensorAddr) && sensors.isConnected(tempSensorAddr)) {
        tempSensorStatus = true;
        sensors.setResolution(TEMP_RESOLUTION);
        sensors.setWaitForConversion(true);                 // library will delay according to DS18B20 datasheet i.e. 750ms at 12bit accuracy
        sensors.requestTemperatures();
        temperature = sensors.getTempC(tempSensorAddr);
        sensors.setWaitForConversion(false);
    } else {
        tempSensorStatus = false;
    }
    #endif
}


// TBA possible crc8 verification
void processSerialCommand() {
    switch(serialBuffer[0]) {
        case 0xF0:  switch(serialBuffer[1]) {               // GET request by master
                        case 0xF1:  Serial.write((byte*) &temperature, sizeof(float));
                                    break;
                        case 0xF2:  Serial.write(getGFA());
                                    break;
                        case 0xF3:  Serial.write(getMotor());
                                    break;
                        case 0xF4:  Serial.write(tempSensorStatus);
                                    break;
                        default:    break;
                    }
        case 0xF1:  //Serial.print(" SET ");
                    //float * dataBuffer;
                    //dataBuffer =  (float *) &(serialBuffer[2]);
                    switch(serialBuffer[1]) {               // SET request by master
                        case 0xF1:  temp_soll = *((float *) &(serialBuffer[2]));
                                    break;
                        case 0xF2:  setGFA(serialBuffer[2]);
                                    break;
                        case 0xF3:  setMotor(serialBuffer[2]);
                                    break;
                        default: break;
                    }
        default:    //Serial.print(" FAIL ");
                    break;
    }
}

/* serialEvent() is an interrupt routine called when incoming serial data ist available */

void serialEvent() {
    int count = 0;
    while((Serial.available() > 0) && (count < SERIAL_BUFFER_SIZE)) {
        serialBuffer[count] = Serial.read();
        count++;
    }
    
    while(count < SERIAL_BUFFER_SIZE) {
        serialBuffer[count] = 0;
        count++;
    }
//  for(int i=0; i < SERIAL_BUFFER_SIZE; i++) {
//    Serial.print(" 0x");
//    Serial.print(serialBuffer[i], HEX);
//  }
    processSerialCommand();
//  Serial.println();
    
}

boolean getGFA() {
    if(digitalRead(RELAY_GFA) == RELAY_OFF) {
        return false;
    } else {
        return true;
    }
}

boolean getMotor() {
    if(digitalRead(RELAY_MOTOR) == RELAY_OFF) {
        return false;
    } else {
        return true;
    }
}

void setGFA(boolean on) {
    if(on) {
        digitalWrite(RELAY_GFA, RELAY_ON);
    } else {
        digitalWrite(RELAY_GFA, RELAY_OFF);
    }
}

void setMotor(boolean on) {
    if(on) {
        digitalWrite(RELAY_MOTOR, RELAY_ON);
    } else {
        digitalWrite(RELAY_MOTOR, RELAY_OFF);
    }
}

void twoLevelController() {
    if(temperature <= (temp_soll-DELTA_FIRST_LIMIT)) {      // if temperature is more than DELTA_FIRST_LIMIT below set temperature, overshooting protection will be enabled
        setGFA(true);
        overshooting = true;
    }
    
    if(getGFA() && overshooting && (temperature >= (temp_soll-DELTA_TEMP_FIRST))) {     // if overshooting protection is enabled and temperature is less than DELTA_TEMP_FIRST below set temperature, heating stops
        setGFA(false);
//        overshooting = false;
        timerGFA = millis();
    }
    
    if(!getGFA() && overshooting && (millis() >= timerGFA+DELTA_TIME_FIRST*1000)) {     // continues heating after DELTA_TIME_FIRST seconds for overshooting protection
        setGFA(true);
        overshooting = false;
        timerGFA = millis();
    }
    
    if(!getGFA() && !overshooting && (temperature <= (temp_soll-DELTA_TEMP_LOW)) && (millis() >= timerGFA+DELTA_TIME*1000)) {   // two-level-controller lower limit
        setGFA(true);
        overshooting = false;
        timerGFA = millis();
    }
    
    if(getGFA() && (temperature >= (temp_soll+DELTA_TEMP_HIGH)) && (millis() >= timerGFA+DELTA_TIME*1000))  {   // two-level-controller upper limit
        setGFA(false);
        overshooting = false;
        timerGFA = millis();
    }
}
