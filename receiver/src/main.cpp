/**
 * Receiver.
 *
 */
#include "Arduino.h"
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

#define DEBUG 0
#define DEBUGSERIAL Serial

#if DEBUG
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINT_LN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINT_LN(x)
#endif


void parseData(Message *msg);
void constructAckMsg(Message *msg); 

// placed in config.h
extern byte addresses[][6];
RF24 radio(10, 9); // CE, CSN

uint8_t speed = 128;

struct bldcMeasure measuredVal;

struct remotePackage remote;

unsigned long count = 0;

char hello [] = "NO DATA";
Message noData;

void setup() {
  Serial.begin(115200);
  SetSerialPort(&SERIALIO);

  convertToBytes(hello, noData.payload, sizeof(hello));
  noData.payloadLength = sizeof(noData.payload);
  noData.dataType = SK8_MESSAGE;

  delay(1000);

  // setup radio
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);

  radio.openReadingPipe(1, addresses[RECEIVER_ADDRESS_INDEX]);
  radio.enableAckPayload();

  radio.setRetries(5,15);
  // the radio listening for data
  radio.startListening();

}

bool receiveMsg(Message *msg) {
   

   if(radio.available()){
      radio.read( msg, sizeof(*msg) );             // Get the payload
      parseData(msg);

      if (VescUartGetValue(measuredVal)) {
        // SerialPrintFocBoxUnity(measuredVal, &DEBUGSERIAL);
        Message ackMsg;
        constructAckMsg(&ackMsg);      
        radio.writeAckPayload(1, &ackMsg, sizeof(ackMsg));
      } else {
        DEBUG_PRINT_LN("Failed to get data from VESC");
        radio.writeAckPayload(1, &noData, sizeof(noData));
      }

      
      //radio.writeAckPayload(1, msg, sizeof(*msg));   
      return true;
  }

  return false;
}


void loop() {
  Message msg;
  receiveMsg(&msg);
}

void constructAckMsg(Message *msg) {
  RequiredReadings readings;
  readings.ampHours = measuredVal.ampHours;
  readings.rpm1 = measuredVal.rpm;
  readings.rpm2 = measuredVal.rpm2;
  readings.wattHoursCharged = measuredVal.wattHoursCharged;
  readings.inputVoltage = measuredVal.inpVoltage;
  // DEBUGSERIAL.print("Amp Hours Charged: ");
  // DEBUGSERIAL.print(readings.ampHoursCharged);
  // DEBUGSERIAL.print(" RPM1: ");
  // DEBUGSERIAL.print(readings.rpm1);
  // DEBUGSERIAL.print(" RPM2: ");
  // DEBUGSERIAL.print(readings.rpm2);
  // DEBUGSERIAL.print(" Watt Hours Charged: ");
  // DEBUGSERIAL.print(readings.wattHoursCharged);
  // DEBUGSERIAL.print(" Avg Input Current: ");
  // DEBUGSERIAL.print(readings.inputCurrent);

  convertToMessage(msg, &readings);
}

void parseData(Message *msg) {
  char message[64] = "";
  
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

      remote.valXJoy = speed;
      remote.valYJoy = speed;
      remote.valUpperButton = false;
      remote.valLowerButton = false;
      
      VescUartSetNunchukValues(remote);
      // TODO: implement MIA message
      break;
    }
    default: {
      DEBUG_PRINT_LN("Unable to parse message!");
      break;
    }
  }
}