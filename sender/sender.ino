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

#ifdef __cplusplus
  extern "C" {
#endif

#include <esk8_model.h>
#include <config.h> // should be created. 

#ifdef __cplusplus
  }
#endif


#define DEBUG

#ifdef DEBUG
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINT_LN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINT_LN(x)
#endif

// placed in config.h
extern byte addresses[][6];

RF24 radio(7, 8); // CSN, CE
                    // D9, D10  
                    
const int spriteWidth = sizeof(heartImage);

const byte Y_AXIS_PIN = 15; //A1
const byte X_AXIS_PIN = 14; //A0

// Click on the analog thumbstick
const byte MODE_PIN = 2; //D2

int rawAnalogValueYAxis = 0;
int convertedValueYAxis = 0;

int rawAnalogValueXAxis = 0;
int convertedValueXAxis = 0;

volatile byte currentMode = 0;
volatile unsigned long previousTime = 0;
const unsigned long DEBOUNCE_THRESHOLD = 250;

const byte DEFAULT_MODE = 0;
const byte SPEED_MODE = 1;
const byte BATTERY_MODE = 2;
const byte SETTINGS_MODE = 3;

void setup() {
    /* Select the font to use with menu and all font functions */
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_128x32_i2c_init();
    
    Serial.begin(115200); 
    analogReference(EXTERNAL);
    pinMode(MODE_PIN, INPUT_PULLUP);
    delayMicroseconds(50);
    attachInterrupt(digitalPinToInterrupt(MODE_PIN), changeMode, FALLING);
    
    // setup radio
    radio.begin();
    radio.setPALevel(RF24_PA_LOW);
    radio.openWritingPipe(addresses[0]);
    //radio.openReadingPipe(1,addresses[1]);


    // Start the radio listening for data
    radio.stopListening();


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
    delay(200);
    
    char message[] = "Hello";
    Message msg = {MESSAGE, sizeof(message)};
    convertToBytes(message, msg.payload, sizeof(message));
    
    if(!radio.write( &msg, sizeof(msg)) ) {
      DEBUG_PRINT_LN("Failed.");
    }
  }
  ssd1306_normalMode();
}

void loop() {
  
  calculateAnalogInputs();
  ssd1306_clearScreen();
  drawBars();
  drawCurrentMode();
  sendData();
  delay(10);
}

void sendData() {
  Message msg = {SPEED, sizeof(convertedValueYAxis)};
  setSpeedValue(&msg, convertedValueYAxis);

  if(!radio.write( &msg, sizeof(msg)) ) {
    DEBUG_PRINT_LN("Failed.");
  }
}

void calculateAnalogInputs() {
  rawAnalogValueYAxis = analogRead(Y_AXIS_PIN); 
  // 1023/4 gives us range from 0-255
  convertedValueYAxis = rawAnalogValueYAxis/4;
  
  rawAnalogValueXAxis = analogRead(X_AXIS_PIN);
  convertedValueXAxis = rawAnalogValueXAxis/4;
}


static void drawBars() {
  uint8_t barY = convertedValueYAxis/2;
  ssd1306_drawHLine(0, 8, barY);

  uint8_t barX = convertedValueXAxis/2;
  ssd1306_drawHLine(0, 10, barX);

  drawHeart(barY);
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
