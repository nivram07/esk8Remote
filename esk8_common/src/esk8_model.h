#ifndef __ESK8_MODEL_H__
#define __ESK8_MODEL_H__

#include <Arduino.h>

void convertToBytes(char *source, byte *destination, uint8_t length);
void convertToCharArr(byte *source, char *destination, uint8_t length);

typedef enum DATA_TYPE {MESSAGE, SPEED} DATA_TYPE;

typedef struct Message {
  DATA_TYPE dataType;
  uint8_t payloadLength;
  byte payload[64]; // fix to 64
} Message;

#endif
