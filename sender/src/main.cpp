/**
 * Sender.
 * Code to be put on the remote.
 * 
 * Nano/Atmega328 PINS: connect LCD to A4/A5 (i2c)
 * 
 */
#include "ssd1306.h"
#include "ssd1306_1bit.h"
#include "bitmaps.h"
#include "logo.h"
#include <SPI.h>
#include "RF24.h"
#include <nRF24L01.h>
#include <VescUart.h>
#include <datatypes.h>

#ifdef __cplusplus
  extern "C" {
#endif

#include <esk8_model.h>
#include <config.h> // should be created. 

#ifdef __cplusplus
  }
#endif

#define DEBUG 1
#define DEBUGSERIAL Serial

#if DEBUG
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINT_LN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINT_LN(x)
#endif

// placed in config.h
extern byte addresses[][6];

RF24 radio(10, 9); // CE, CSN
                    // D7, D8  
                    
const int spriteWidth = sizeof(heartImage);

const byte SPEED_PIN = 16; //A2

// Click on the analog thumbstick
const byte MODE_PIN = 2; //D2

int rawSpeedValue = 0;
int convertedSpeedValue = 0;

int upperLimitSpeed = 1023;
int lowerLimitSpeed = 0;

volatile byte currentMode = 0;
volatile unsigned long previousTime = 0;
const unsigned long DEBOUNCE_THRESHOLD = 250;

const byte DEFAULT_MODE = 0;
const byte SPEED_MODE = 1;
const byte BATTERY_MODE = 2;
const byte SETTINGS_MODE = 3;


const double wheelDiametermm = 80.0;
const double gearRatio = 36/16; // 36T, 16T pulley from diyelectricskateboard;


double mph = 0.0;

double battPercent = 0.0;
const double peakVoltage = 50.4; // based on a 12s4p battery
const double cutoffVoltage = 37.2;

// message from receiver
struct bldcMeasure receivedMsg;

void parseData(Message *msg);
static void drawHeart(uint8_t position);
static void drawCurrentMode();
static void drawBars();
void calculateAnalogInputs();
boolean sendData();

static void startUpScreen();
static void drawNoSignal();

void calculateBattPercent();
static void drawBatteryPercent();
void changeMode();
void calculateMPH();

void setup() {
    /* Select the font to use with menu and all font functions */
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_128x32_i2c_init();
    
    Serial.begin(115200); 
    analogReference(DEFAULT);
    pinMode(MODE_PIN, INPUT_PULLUP);

    delayMicroseconds(50);
    attachInterrupt(digitalPinToInterrupt(MODE_PIN), changeMode, FALLING);
    
    // setup radio
    radio.begin();
    radio.setPALevel(RF24_PA_HIGH);
    radio.setDataRate(RF24_250KBPS);
    radio.enableAckPayload();
    radio.setRetries(5,15);
    radio.openWritingPipe(addresses[RECEIVER_ADDRESS_INDEX]);

    delay(500);

    previousTime = millis();
    startUpScreen();
    DEBUG_PRINT_LN("Setup done.");
}

void changeMode() {
  if (millis() - previousTime > DEBOUNCE_THRESHOLD) {
    if (currentMode >= SETTINGS_MODE) {
      currentMode = DEFAULT_MODE;
    } else {
       currentMode++;
    }
    previousTime = millis();
  }
}

static void startUpScreen() {
  for (int i = 0; i < 3; i++) {
    
    ssd1306_clearScreen();
    ssd1306_drawBitmap(0, 0, 128, 32, logo);
    ssd1306_normalMode();
    delay(800);
    ssd1306_invertMode();
  }
  ssd1306_normalMode();
}

void loop() {
  
  calculateAnalogInputs();
  
  if (sendData()) {
    drawBars();
    drawCurrentMode();
    drawBatteryPercent();
  } else {
    drawNoSignal();
  }
   delay(5);
}

boolean sendData() {
  Message msg = {SK8_SPEED, sizeof(convertedSpeedValue)};
  setSpeedValue(&msg, convertedSpeedValue);
  bool sendSuccess = radio.write( &msg, sizeof(msg));
  if( sendSuccess ) {
    if (radio.isAckPayloadAvailable()) {
      Message msgReceived;
      radio.read( &msgReceived, sizeof(msgReceived) );             // Get the payload
      parseData(&msgReceived);
    }
    return true;
  }
    
  DEBUG_PRINT_LN("Failed to send.");
  return false;
}

void calculateAnalogInputs() {

  rawSpeedValue = analogRead(SPEED_PIN);
  convertedSpeedValue = map(rawSpeedValue, lowerLimitSpeed, upperLimitSpeed, 0, 255);  
}


static void drawBars() {
  
  uint8_t barSpeed = convertedSpeedValue/2;
  if (barSpeed <= 0) barSpeed = 0;
  else if (barSpeed >= 128) barSpeed = 128;
  ssd1306_clearScreen();
  ssd1306_drawHLine(0, 8, barSpeed);

  drawHeart(barSpeed);
}

static void drawNoSignal() {
  
  ssd1306_clearScreen();
  ssd1306_printFixed(16, 16, "NO SIGNAL", STYLE_BOLD);
}

static void drawCurrentMode() {
 
  ssd1306_clearScreen();
  switch(currentMode) {
    case DEFAULT_MODE: {
        ssd1306_printFixed(0, 16, "DEF", STYLE_BOLD);
        break;
    }
    case SPEED_MODE: {
      ssd1306_printFixed(0, 16, "SPD", STYLE_BOLD);
        break;
    }
    case BATTERY_MODE: {
      ssd1306_printFixed(0, 16, "BAT", STYLE_BOLD);
        break;
    }
    case SETTINGS_MODE: {
      ssd1306_printFixed(0, 16, "SET", STYLE_BOLD);
        break;
    }
  }
}

static void drawHeart(uint8_t position) {
    /* Declare variable that represents our sprite */
    SPRITE sprite;
    if (position >= 120) {
      sprite = ssd1306_createSprite( 120, 0, spriteWidth, fck );
    } else {
      sprite = ssd1306_createSprite( position, 0, spriteWidth, fck );
    }
    /* Erase sprite on old place. The library knows old position of the sprite. */
    sprite.eraseTrace();
    /* Draw sprite on new place */
    sprite.draw();
}

static void drawBatteryPercent() {
  ssd1306_clearScreen();
  char battPercentStr[8];
  dtostrf(battPercent, 1, 2, battPercentStr);
  ssd1306_printFixed(32, 16, battPercentStr, STYLE_BOLD);
}

void calculateMPH(float rpm) {
  mph = rpm * 60.0 * wheelDiametermm / 1000.0 * PI * 0.000621371 / 10;
}


void calculateBattPercent(float inputVoltage, float peakVoltage, float  cutOffVoltage) {
  battPercent = (inputVoltage - cutOffVoltage)/(peakVoltage - cutOffVoltage) * 100;
}

void parseData(Message *msg) {
  char message[64] = "";
  uint8_t speed;
  switch (msg->dataType) {

    case SK8_MESSAGE: {
      convertToCharArr(msg->payload, message, msg->payloadLength);
      DEBUG_PRINT("message: ");
      DEBUG_PRINT_LN(message);  
      break;
    }
    case SK8_SPEED: {
      speed = getSpeedValue(msg);
      DEBUG_PRINT("speed: ");
      DEBUG_PRINT_LN(speed);  
      // TODO: implement MIA message
      break;
    }
    case SK8_TEL_REQUIRED_READINGS: {
      //DEBUGSERIAL.println("Readings received.");
      RequiredReadings readings;
      int checkSize = convertToRequiredReadings(msg, &readings);
      calculateBattPercent(readings.inputVoltage, peakVoltage, cutoffVoltage);
      calculateMPH(min(readings.rpm1, readings.rpm2));
      //DEBUGSERIAL.print("payload size: "); DEBUG_PRINT_LN(msg->payloadLength);
      //DEBUG_PRINT("last index value: ");
      //DEBUG_PRINT_LN(checkSize);
      //DEBUG_PRINT("Amp Hours Charged: ");
      //DEBUG_PRINT_LN(readings.ampHoursCharged);
      DEBUG_PRINT("RPM1: ");
      DEBUG_PRINT(readings.rpm1);
      DEBUG_PRINT(" RPM2: ");
      DEBUG_PRINT(readings.rpm2);
      DEBUG_PRINT(" IN_VOLTAGE: ");
      DEBUG_PRINT(readings.inputVoltage);
      DEBUG_PRINT(" AMP_HRS: ");
      DEBUG_PRINT(readings.ampHours);
      DEBUG_PRINT(" MPH1: ");
      DEBUG_PRINT_LN(mph);
      //DEBUG_PRINT("Watt Hours Charged: ");
      //DEBUG_PRINT_LN(readings.wattHoursCharged);
      //DEBUG_PRINT("Avg Input Current: ");
      //DEBUG_PRINT_LN(readings.inputCurrent);
      break;
    }
    default: {
      break;
    }
  }
}
