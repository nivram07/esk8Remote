

/**
 * Receiver.
 * 
 * Nano/Atmega328 PINS: connect LCD to A4/A5 (i2c)
 * 
 */
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

void setup() {
  Serial.begin(115200);

  // setup radio
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);

  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);

  // Start the radio listening for data
  radio.startListening();
}

void loop() {
    Message msg;
    if( radio.available()){
        while (radio.available()) {                    // Variable for the received timestamp
          radio.read( &msg, sizeof(msg) );             // Get the payload
        }
        parseData(&msg);
   }
}

void parseData(Message *msg) {
  char message[64] = "";
  uint8_t speed;
  switch (msg->dataType) {

    case MESSAGE: {
      convertToCharArr(msg->payload, message, msg->payloadLength);
      DEBUG_PRINT("message: ");
      DEBUG_PRINT_LN(message);  
      break;
    }
    case SPEED: {
      speed = getSpeedValue(msg);
      DEBUG_PRINT("speed: ");
      DEBUG_PRINT_LN(speed);  
      // TODO: send speed to the vesc
      // TODO: implement MIA message
      break;
    }
    default: {
      break;
    }
  }
}
