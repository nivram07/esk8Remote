#ifndef __ESK8_MODEL_H__
#define __ESK8_MODEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "Arduino.h"

#ifdef __cplusplus
  }
#endif

typedef enum DATA_TYPE {
    SK8_MESSAGE,
    SK8_SPEED,
    SK8_TEL_AMP_HR_CHARGED,
    SK8_TEL_RPM1,
    SK8_TEL_RPM2,
    SK8_TEL_WATT_HRS_CHARGED,
    SK8_TEL_AVG_INPUT_CURRENT,
    SL8_TEL_REQUIRED_READINGS
} DATA_TYPE;


typedef struct RequiredReadings {
  float ampHourCharged;
  float rpm1;
  float rpm2;
  float watHoursCharged;
  float inputCurrent;
} RequiredReadings;

typedef struct Message {
  DATA_TYPE dataType;
  uint8_t payloadLength;
  byte payload[24];
} Message;

typedef union FloatPayload {
  float value;
  byte valueInBytes[4];
} FloatPayload;


void convertToBytes(char *source, byte *destination, uint8_t length);
void convertToCharArr(byte *source, char *destination, uint8_t length);

void setSpeedValue(Message *msg, uint8_t value);
uint8_t getSpeedValue(Message *msg);


#endif
