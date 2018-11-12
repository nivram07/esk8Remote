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

const int spriteWidth = sizeof(heartImage);

const byte yAxisPin = 14; //A0
const byte modePin = 2; //D2

int rawAnalogValueYAxis = 0;
int convertedValueYAxis = 0;

volatile byte currentMode = 0;
volatile unsigned long previousTime = 0;
const unsigned long debounceThreshold = 250;

const byte DEFAULT_MODE = 0;
const byte SPEED_MODE = 1;
const byte BATTERY_MODE = 2;
const byte SETTINGS_MODE = 3;

void setup() {
    /* Select the font to use with menu and all font functions */
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_128x32_i2c_init();

    Serial.begin(9600); 
    analogReference(EXTERNAL);
    pinMode(yAxisPin, INPUT_PULLUP);
    
    pinMode(modePin, INPUT_PULLUP);
    delayMicroseconds(50);
    attachInterrupt(digitalPinToInterrupt(modePin), changeMode, FALLING);
    previousTime = millis();
    startUpScreen();
}

void changeMode() {
  if (millis() - previousTime > debounceThreshold) {
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
   }
   ssd1306_normalMode();
}

void loop() {
  
  rawAnalogValueYAxis = analogRead(yAxisPin);
  
  // 1023/4 gives us range from 0-255
  convertedValueYAxis = rawAnalogValueYAxis/4;
  Serial.println(convertedValueYAxis/2);
  drawScreen();
  Serial.print("MODE: ");
  Serial.println(currentMode);
  drawCurrentMode();
  delay(10);
}


static void drawScreen() {
  ssd1306_clearScreen();
  uint8_t bar = convertedValueYAxis/2;
  ssd1306_drawHLine(0, 8, bar);
  drawHeart(bar);
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
