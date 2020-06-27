#ifndef __ESK8_MODEL_H__
#define __ESK8_MODEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "Arduino.h"
#include "local_datatypes.h"

#ifdef __cplusplus
  }
#endif


void convertToBytes(char *source, byte *destination, uint8_t length);
void convertToCharArr(byte *source, char *destination, uint8_t length);

void setSpeedValue(Message *msg, uint8_t value);
uint8_t getSpeedValue(Message *msg);

float getFloatValue(int *sourceIndex, byte* source);
void injectToPayload(int *currentIndex, byte* payload, FloatPayload *item);

int convertToRequiredReadings(Message *source, RequiredReadings *target); 
void convertToMessage(Message *target, RequiredReadings *source); 
#endif
