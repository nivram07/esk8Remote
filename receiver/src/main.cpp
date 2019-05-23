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

#define SERIALIO Serial1
#define DEBUGSERIAL Serial

void parseData(Message *msg);
void SerialPrint2(const struct bldcMeasure values);

// placed in config.h
extern byte addresses[][6];
RF24 radio(5, 6); // CE, CSN


struct bldcMeasure measuredVal;

struct remotePackage remote;

unsigned long count = 0;

Message ackMsg;

void setup() {

  Serial.begin(115200);
  SERIALIO.begin(115200);
  SetSerialPort(&SERIALIO);
  DEBUGSERIAL.begin(115200);
  delay(1000);

  // setup radio
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_2MBPS);

  radio.openReadingPipe(1, addresses[RECEIVER_ADDRESS_INDEX]);
  radio.enableAckPayload();

  radio.setRetries(5,15);
  // the radio listening for data
  radio.startListening();
  DEBUGSERIAL.println("Setup complete");
}

bool receiveMsg(Message *msg) {
   char hello [] = "hello";
   convertToBytes(hello, ackMsg.payload, sizeof(hello));
   ackMsg.payloadLength = sizeof(ackMsg.payload);
   ackMsg.dataType = SK8_MESSAGE;

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
  DEBUGSERIAL.println("Reading data");
   if (VescUartGetValue(measuredVal)) {
      DEBUGSERIAL.print("Received data\n");
      SerialPrint2(measuredVal);
    } else {
      DEBUGSERIAL.println("Failed to get data!");
    }
      // delay(1000);

}

void parseData(Message *msg) {
  char message[64] = "";
  DEBUGSERIAL.println("Message received from remote!");
  uint8_t speed;
  switch (msg->dataType) {

    case SK8_MESSAGE: {
      convertToCharArr(msg->payload, message, msg->payloadLength);
      DEBUGSERIAL.print("message: ");
      DEBUGSERIAL.println(message);
      break;
    }
    case SK8_SPEED: {
      speed = getSpeedValue(msg);
      DEBUGSERIAL.print("speed: ");
      DEBUGSERIAL.println(speed);
      remote.valXJoy = 0;
      remote.valYJoy = speed;
      remote.valUpperButton = false;
      remote.valLowerButton = false;
      
      VescUartSetNunchukValues(remote);
      // TODO: send speed to the vesc
      // TODO: implement MIA message
      break;
    }
    default: {
      break;
    }
  }
}



void SerialPrint2(const struct bldcMeasure values) {
   Serial.print("tempFetFiltered: ");  Serial.println(values.tempFetFiltered);
   Serial.print("tempFetFiltered: ");  Serial.println(values.tempFetFiltered2);
   Serial.print("tempMotorFiltered:");  Serial.println(values.tempMotorFiltered);
   Serial.print("tempMotorFiltered2:");  Serial.println(values.tempMotorFiltered2);
   Serial.print("avgMotorCurrent: ");  Serial.println(values.avgMotorCurrent);
   Serial.print("avgMotorCurrent2: ");  Serial.println(values.avgMotorCurrent2);
   Serial.print("avgInputCurrent: ");  Serial.println(values.avgInputCurrent);
   Serial.print("avgId:     ");  Serial.println(values.avgId);
   Serial.print("avgId2:     ");  Serial.println(values.avgId2);
   Serial.print("avgIq:     ");  Serial.println(values.avgIq);
   Serial.print("avgIq2:     ");  Serial.println(values.avgIq2);
   Serial.print("dutyNow:     ");  Serial.println(values.dutyNow);
   Serial.print("dutyNow2:     ");  Serial.println(values.dutyNow2);
   Serial.print("rpm:       ");  Serial.println(values.rpm);
   Serial.print("rpm2:       ");  Serial.println(values.rpm2);
   Serial.print("inpVoltage:    ");  Serial.println(values.inpVoltage);
   Serial.print("ampHours:    ");  Serial.println(values.ampHours);
   Serial.print("ampHoursCharged: ");  Serial.println(values.ampHoursCharged);
   Serial.print("wattHours:   ");  Serial.println(values.wattHours);
   Serial.print("wattHoursCharged:   ");  Serial.println(values.wattHoursCharged);
   Serial.print("tachometer:    ");  Serial.println(values.tachometer);
   Serial.print("tachometer2:    ");  Serial.println(values.tachometer2);
   Serial.print("tachometerAbs: ");  Serial.println(values.tachometerAbs);
   Serial.print("tachometerAbs2: ");  Serial.println(values.tachometerAbs2);

   Serial.print("faultCode:   ");  Serial.println(values.faultCode);
}
