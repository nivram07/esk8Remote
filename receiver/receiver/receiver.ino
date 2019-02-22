

/**
 * Receiver.
 * 
 * Nano/Atmega328 PINS: connect LCD to A4/A5 (i2c)
 * 
 */
#include <SPI.h>
#include "RF24.h"
#include <nRF24L01.h>
#include <SoftwareSerial.h>
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

#define DEBUG

#ifdef DEBUG
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINT_LN(x)  Serial.println (x)
 #define VESCSERIAL
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINT_LN(x)
 #define VESCSERIAL         Serial 
#endif
// to VESC

// placed in config.h
extern byte addresses[][6];
RF24 radio(7, 8); // CSN, CE
                    // D9, D10

struct bldcMeasure measuredVal;

unsigned long count = 0;

Message ackMsg;

void setup() {

#ifdef DEBUG
  Serial.begin(115200);
#else
  VESCSERIAL.begin(115200);
  SetSerialPort(&VESCSERIAL);
#endif

  delay(1000);

  // setup radio
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_2MBPS);

  radio.openReadingPipe(1, addresses[0]);
  radio.enableAckPayload();    
   
    radio.setRetries(5,15);
  // Start the radio listening for data
   radio.startListening();
}

bool receiveMsg(Message *msg) {
  
   
   char hello [] = "hello";
   convertToBytes(hello, ackMsg.payload, sizeof(hello));
   ackMsg.payloadLength = sizeof(ackMsg.payload);
   ackMsg.dataType = MESSAGE;
   
   if( radio.available()){
        radio.read( msg, sizeof(*msg) );             // Get the payload
      parseData(msg);
      radio.writeAckPayload(1, &ackMsg, sizeof(ackMsg)); 
      return true;
   }
   return false;
}


void loop() {
  Message msg;
  receiveMsg(&msg);

//  if (count >= 250000) {
//   if (VescUartGetValue(measuredVal)) {
//     if (!radio.write(&measuredVal, sizeof(measuredVal))) {
//        //Error here
//      }
//    }
//   count = 0; 
//  } else {
//    count++;
//  }
  
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
