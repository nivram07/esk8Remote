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
    SK8_TEL_RPM,
    SK8_TEL_WATT_HRS_CHARGE,
    SK8_TEL_AVG_MOTOR_CURRENT

} DATA_TYPE;

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

void setAmpHrCharged(Message *msg, float value);
float getAmpHrCharged(Message *msg);


#endif
