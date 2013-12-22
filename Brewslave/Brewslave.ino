
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LCD5110_Basic.h>


/* PIN DEFINITIONS */

#define ONE_WIRE_BUS 2                                    // DS18B20 sensor data pin

#define RELAY_GFA 12                                      // GFA relay pin
#define RELAY_MOTOR 11                                    // motor relay pin

#define LCD_RST 6
#define LCD_CE 7
#define LCD_DC 5
#define LCD_DIN 4
#define LCD_CLK 3


/* CONSTANTS */

#define RELAY_ON 0
#define RELAY_OFF 1

extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

extern uint8_t img_line[];
extern uint8_t test_position[];

#define DELTA_FIRST_LIMIT 5                               // lower offset to set temperature [C] at which overshooting is considered (> DELTA_TEMP_FIRST)
#define DELTA_TEMP_FIRST 1.5                              // lower offset to set temperature [C] at which heating stops to avoid overshooting (> DELTA_TEMP_LOW)
#define DELTA_TIME_FIRST 60                               // break time [s] to avoid overshooting
#define DELTA_TEMP_HIGH 0.5                               // upper limit temperature offset [C] for two-level-controller
#define DELTA_TEMP_LOW 0.5                                // lower limit temperature offset [C] for two-level-controller
#define DELTA_TIME 60                                     // delay time between two-level-controller switches


/* SETTINGS */

#define TEMP_RESOLUTION 12
//#define ONEWIRE_CRC8_TABLE 1                              // lookuptable for crc8 check, fastert but more memory nessessary

#define SERIAL_BUFFER_SIZE 6                              // define the size (amount of bytes) of the serial buffer


/* INITIALIZE INSTANCES */

OneWire ourWire(ONE_WIRE_BUS);                            // initialize OneWire instance
DallasTemperature sensors(&ourWire);                      // initialize DallasTemperature Library with OneWire instance
LCD5110 myGLCD(LCD_CLK, LCD_DIN, LCD_DC, LCD_RST, LCD_CE);// initialize Nokia 5110 gLCD instance


/* GLOBAL VARIABLES */

byte tempSensorAddr[8];                                   // cache for temperature sensor address
boolean tempSensorStatus = false;

float temperature = -999.1337;                           
float temp_soll = 42.4242;

byte serialBuffer [SERIAL_BUFFER_SIZE];

boolean overshooting = false;
unsigned int timerGFA = -1000 * DELTA_TIME;


/* TEST VARIABLES */

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

float start = 0;
float ende  = 0;


/* FORWARD DECLARATIONS */
void resetTempSensor();


void setup() 
{ 
  noInterrupts();                                         // disable all interrupts

  /* disable relays at startup */

  digitalWrite(RELAY_GFA, RELAY_OFF);
  digitalWrite(RELAY_MOTOR, RELAY_OFF);
  pinMode(RELAY_GFA, OUTPUT);
  pinMode(RELAY_MOTOR, OUTPUT);

  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  myGLCD.print("Brewmeister0.3", CENTER, 0);
  myGLCD.print("Loading...", CENTER, 24);

  sensors.begin(); 
  ourWire.reset_search();
  ourWire.search(tempSensorAddr);
  
  resetTempSensor();
  
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

  for(int i=1; i<6; i++) {
      myGLCD.clrRow(i, 0, 83);
  }
}

void loop() 
{
  noInterrupts();
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
  interrupts();

  
  
  
  
  
  
  
  
  //  noInterrupts();
  //  myGLCD.drawBitmap(0,8, img_line, 84, 8);
  //  myGLCD.drawBitmap(0, 8, test_position, 84,24);
  //
  //  //myGLCD.clrRow(5, 0, 15);
  //  myGLCD.setFont(SmallFont);
  //  myGLCD.print("--", LEFT, 40);
  //  interrupts();
  //String test = dtostrf(temp_soll,3,0,StrBuffer);
  //myGLCD.print(StrBuffer, LEFT, 76);
  delay(1000);
}


ISR(TIMER1_COMPA_vect)                                  // timer compare interrupt service routine
{
//  start = millis();

  if(sensors.isConnected(tempSensorAddr) && tempSensorStatus) {
    Serial.print("T=");
    Serial.println(temperature);
    temperature = sensors.getTempC(tempSensorAddr);    // get temperature from sensor
    sensors.requestTemperatures();                     // request new temperature conversion
  } else {
    tempSensorStatus = false;
    ourWire.reset_search();
    ourWire.search(tempSensorAddr);
    resetTempSensor();
  }
  
//  ende = millis();
//  Serial.print(ende - start);
}


/* reconfigures temperature sensor, neccessary after lost connection */

void resetTempSensor() {
  if((tempSensorAddr[0] == 0x28) && sensors.validAddress(tempSensorAddr) && sensors.isConnected(tempSensorAddr)) {
    tempSensorStatus = true;
    sensors.setResolution(TEMP_RESOLUTION);
    sensors.setWaitForConversion(true);
    sensors.requestTemperatures();
    temperature = sensors.getTempC(tempSensorAddr);
    sensors.setWaitForConversion(false);
  } else {
    tempSensorStatus = false;
  }
}

void processSerialCommand() {
  switch(serialBuffer[0]) {
    case 0xF0:  //Serial.print(" GET ");
                switch(serialBuffer[1]) {              // 
                  case 0xF1: Serial.write((byte*) &temp_soll,sizeof(float));
                             break;
                  case 0xF2: 
                  case 0xF3:
                  case 0xF4:
                  default: break;
                }
    case 0xF1:  //Serial.print(" SET ");
                byte * dataBuffer;
                dataBuffer =  (byte *) &(serialBuffer[2]);
                switch(serialBuffer[1]) {
                  case 0xF1: temp_soll = *((float *) dataBuffer);
                             break;
                  case 0xF2:
                  case 0xF3:
                  default: break;
                }
    default:    //Serial.print(" FAIL ");
                break;
  }
}

/* serialEvent() is an interrupt called when incoming serial data ist available */

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
  

  //  myGLCD.clrScr();
  //  int i = 0;
  //  
  //  
  //  
  //  while(Serial.available() > 0) {
  //    byte data = Serial.read();
  //    if(data == 0xF1) {
  //      myGLCD.print("ja",LEFT,i*8);
  //    } else if (data == 0xF0) {
  //      myGLCD.print("nein",LEFT,i*8);
  //    } else {
  //      myGLCD.print("pups",LEFT,i*8);
  //    }
  //    i++;
  //  }
  //      
  //  byte inChar1 = Serial.read();
  ////  byte inChar2 = Serial.read();
  //
  //  char bla[2];
  //  snprintf(bla,2,"%x",inChar1);
  //  myGLCD.clrScr();
  //  myGLCD.print(bla,i*12,0);
  //  i++;
  //  }

  //  while (Serial.available()) {
  //    // get the new byte:
  //    char inChar = (char)Serial.read();
  //    // add it to the inputString:
  //    inputString += inChar;
  //    // if the incoming character is a newline, set a flag
  //    // so the main loop can do something about it:
  //    if (inChar == '\n') {
  //      stringComplete = true;
  //    }
  //  }
  //  Serial.write((byte*) &temp_soll,sizeof(float));
  //  myGLCD.clrScr();
  //  myGLCD.setFont(MediumNumbers);
  //  myGLCD.printNumI(millis(),LEFT,0);


  //  Serial.print(" dies ist ein Serial Event, Zeit: ");
  //  Serial.println(ende-start);
  //  if(temp_soll == -999)
  //  {
  //    myGLCD.clrRow(5, 0, 15);
  //    myGLCD.setFont(SmallFont);
  //    String test = dtostrf(temp_soll,3,0,StrBuffer);
  //    myGLCD.print(StrBuffer, RIGHT, 76);
  //  }
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
  
}

void twoLevelController() {
  if(temperature <= (temp_soll-DELTA_FIRST_LIMIT)) {     // if temperature is more than DELTA_FIRST_LIMIT below set temperature, overshooting protection will be enabled
    setGFA(true);
    overshooting = true;
  }

  if(getGFA() && overshooting && (temperature >= (temp_soll-DELTA_TEMP_FIRST))) {    // if overshooting protection is enabled and temperature is less than DELTA_TEMP_FIRST below set temperature, heating stops
    setGFA(false);
//    overshooting = false;
    timerGFA = millis();
  }
  
  if(!getGFA() && overshooting && (millis() >= timerGFA+DELTA_TIME_FIRST*1000)) {    // continues heating after DELTA_TIME_FIRST seconds for overshooting protection
    setGFA(true);
    overshooting = false;
    timerGFA = millis();
  }
  
  if(!getGFA() && !overshooting && (temperature <= (temp_soll-DELTA_TEMP_LOW)) && (millis() >= timerGFA+DELTA_TIME*1000)) {    // two-level-controller lower limit
    setGFA(true);
    overshooting = false;
    timerGFA = millis();
  }

  if(getGFA() && (temperature >= (temp_soll+DELTA_TEMP_HIGH)) && (millis() >= timerGFA+DELTA_TIME*1000))  {    // two-level-controller upper limit
    setGFA(false);
    overshooting = false;
    timerGFA = millis();
  }
}
