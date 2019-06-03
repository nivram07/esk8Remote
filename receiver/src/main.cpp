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
void constructAckMsg(Message *msg); 

// placed in config.h
extern byte addresses[][6];
RF24 radio(5, 6); // CE, CSN

#define LED1_PIN 7
#define LED2_PIN 8
#define LED3_PIN 9


struct bldcMeasure measuredVal;

struct remotePackage remote;

unsigned long count = 0;

void setup() {

  Serial.begin(115200);
  SERIALIO.begin(115200);
  SetSerialPort(&SERIALIO);
  DEBUGSERIAL.begin(115200);
  delay(1000);

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);

  // setup radio
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_2MBPS);

  radio.openReadingPipe(1, addresses[RECEIVER_ADDRESS_INDEX]);
  radio.enableAckPayload();

  radio.setRetries(5,15);
  // the radio listening for data
  radio.startListening();
  
  // we love blinking
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, LOW);
    delay(500);
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    digitalWrite(LED3_PIN, HIGH);
    delay(500);
  }

  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  DEBUGSERIAL.println("Setup complete");
}

bool receiveMsg(Message *msg) {
   char hello [] = "hello this test length!!";
   Message ackMsg;
   convertToBytes(hello, ackMsg.payload, sizeof(hello));
   ackMsg.payloadLength = sizeof(ackMsg.payload);
   ackMsg.dataType = SK8_MESSAGE;

   if( radio.available()){
      digitalWrite(LED1_PIN, HIGH);
      radio.read( msg, sizeof(*msg) );             // Get the payload
      parseData(msg);

      if (VescUartGetValue(measuredVal)) {
        digitalWrite(LED2_PIN, HIGH);
        // SerialPrintFocBoxUnity(measuredVal, &DEBUGSERIAL);
      } else {
        DEBUGSERIAL.println("Failed to get data!");
      }

      Message msg;
      constructAckMsg(&msg);
      printBytes(msg.payload, msg.payloadLength, &DEBUGSERIAL);
      radio.writeAckPayload(1, &msg, sizeof(ackMsg));
      digitalWrite(LED1_PIN, LOW);
      return true;
   }
   return false;
}


void loop() {
  Message msg;
  receiveMsg(&msg);
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
}

void constructAckMsg(Message *msg) {
  RequiredReadings readings;
  readings.ampHoursCharged = measuredVal.ampHoursCharged;
  readings.rpm1 = measuredVal.rpm;
  readings.rpm2 = measuredVal.rpm2;
  readings.wattHoursCharged = measuredVal.wattHoursCharged;
  readings.inputCurrent = measuredVal.avgInputCurrent;
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
  //DEBUGSERIAL.println("Message received from remote!");
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
      DEBUGSERIAL.print(" speed: ");
      DEBUGSERIAL.println(speed);
      remote.valXJoy = speed;
      remote.valYJoy = speed;
      remote.valUpperButton = false;
      remote.valLowerButton = false;
      
      VescUartSetNunchukValues(remote);
      // TODO: implement MIA message
      break;
    }
    default: {
      DEBUGSERIAL.println("Unable to parse message!");
      break;
    }
  }
}