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

const byte DEAD_MAN_SWITCH_PIN = 9; //D9

int rawSpeedValue = 0;
int convertedSpeedValue = 0;

const unsigned short SET_UPPER_LIMIT_SPEED = 1;
const unsigned short SET_LOWER_LIMIT_SPEED = 0;

int upperLimitSpeed = 1023;
int lowerLimitSpeed = 0;

volatile byte currentMode = 0;
volatile unsigned long previousTime = 0;
const unsigned long DEBOUNCE_THRESHOLD = 250;

const byte DEFAULT_MODE = 0;
const byte SPEED_MODE = 1;
const byte BATTERY_MODE = 2;
const byte SETTINGS_MODE = 3;

// message from receiver
struct bldcMeasure receivedMsg;

void parseData(Message *msg);
static void drawHeart(uint8_t position);
static void drawCurrentMode();
static void drawBars();
void calculateAnalogInputs();
void sendData();

static void startUpScreen();

void changeMode();
void setLimitSpeed(unsigned short mode);


void setup() {
    /* Select the font to use with menu and all font functions */
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_128x32_i2c_init();
    
    Serial.begin(115200); 
    analogReference(DEFAULT);
    pinMode(MODE_PIN, INPUT_PULLUP);
    pinMode(DEAD_MAN_SWITCH_PIN, INPUT_PULLUP);

    delayMicroseconds(50);
    attachInterrupt(digitalPinToInterrupt(MODE_PIN), changeMode, FALLING);
    
    // setup radio
    radio.begin();
    radio.setPALevel(RF24_PA_LOW);
    radio.setDataRate(RF24_250KBPS);
    radio.enableAckPayload();
    radio.setRetries(5,15);
    radio.openWritingPipe(addresses[RECEIVER_ADDRESS_INDEX]);

    ssd1306_clearScreen(); 
    ssd1306_printFixed(0, 16, "HOLD ACCELERATE", STYLE_BOLD);
    delay(5000);
    setLimitSpeed(SET_UPPER_LIMIT_SPEED);
    
    ssd1306_clearScreen(); 
    ssd1306_printFixed(0, 16, "HOLD BRAKE", STYLE_BOLD);
    delay(5000);
    setLimitSpeed(SET_LOWER_LIMIT_SPEED);

    ssd1306_clearScreen();   
    ssd1306_printFixed(0, 16, "DONE CALIBRATING", STYLE_BOLD);
    delay(1500);
    DEBUG_PRINT("MIN: ");
    DEBUG_PRINT(lowerLimitSpeed);
    DEBUG_PRINT( "MAX: ");
    DEBUG_PRINT(upperLimitSpeed);
    DEBUG_PRINT_LN("");
    delay(1500);

    previousTime = millis();
    startUpScreen();
    DEBUG_PRINT_LN("Setup done.");
}

void setLimitSpeed(unsigned short mode) {
  int current = 0;
  for (int i = 0; i < 20; i++) {
    current += analogRead(SPEED_PIN);
    delay(5);
  }
  
  if (mode == SET_UPPER_LIMIT_SPEED) {
    upperLimitSpeed = current/20; // just average it
    DEBUG_PRINT("ACTUAL MAX: ");
    DEBUG_PRINT_LN(upperLimitSpeed);
    if (upperLimitSpeed < 700) upperLimitSpeed = 1023; // adjust if too low
  } else {
    lowerLimitSpeed = current/20;
    if (lowerLimitSpeed > 200) lowerLimitSpeed = 0; // adjust if too high
  }
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
  ssd1306_clearScreen();
  drawBars();
  drawCurrentMode();
  
  sendData();
   delay(5);
}

void sendData() {
  //if (!digitalRead(DEAD_MAN_SWITCH_PIN)) {
    Message msg = {SK8_SPEED, sizeof(convertedSpeedValue)};
    setSpeedValue(&msg, convertedSpeedValue);
    // DEBUG_PRINT("Sending: ");
    // DEBUG_PRINT_LN(convertedSpeedValue);
    bool sendSuccess = radio.write( &msg, sizeof(msg));
    if( sendSuccess ) {
      if (radio.isAckPayloadAvailable()) {
        Message msgReceived;
        radio.read( &msgReceived, sizeof(msgReceived) );             // Get the payload
        parseData(&msgReceived);
      }
    } else {
      DEBUG_PRINT_LN("Failed.");
    }
}

void calculateAnalogInputs() {

  rawSpeedValue = analogRead(SPEED_PIN);
  convertedSpeedValue = map(rawSpeedValue, lowerLimitSpeed, upperLimitSpeed, 0, 255);  
}


static void drawBars() {
  uint8_t barSpeed = convertedSpeedValue/2;
  ssd1306_drawHLine(0, 8, barSpeed);

  drawHeart(barSpeed);
}

static void drawCurrentMode() {
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
      //DEBUGSERIAL.print("payload size: "); DEBUG_PRINT_LN(msg->payloadLength);
      //DEBUG_PRINT("last index value: ");
      //DEBUG_PRINT_LN(checkSize);
      //DEBUG_PRINT("Amp Hours Charged: ");
      //DEBUG_PRINT_LN(readings.ampHoursCharged);
      DEBUG_PRINT("RPM1: ");
      DEBUG_PRINT(readings.rpm1);
      DEBUG_PRINT(" RPM2: ");
      DEBUG_PRINT_LN(readings.rpm2);
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
