#include "esk8_model.h"

void convertToBytes(char *source, byte *destination, uint8_t length) {
  for (int i = 0; i < length; i++) {
    destination[i] = (byte) source[i];
  }
}

void convertToCharArr(byte *source, char *destination, uint8_t length) {
  for (int i = 0; i < length; i++) {
    destination[i] = (char) source[i];
  }
}


void setSpeedValue(Message *msg, uint8_t value) {
  msg->payload[0] = value;
}

void convertToMessage(Message *target, RequiredReadings *source) {
  target->dataType = SK8_TEL_REQUIRED_READINGS;
  int currentIndex = 0;

  
  FloatPayload item;
  item.value = source->ampHours;
  injectToPayload(&currentIndex, target->payload, &item);
  
  item.value = source->rpm1;
  injectToPayload(&currentIndex, target->payload, &item);

  item.value = source->rpm2;
  injectToPayload(&currentIndex, target->payload, &item);
  
  item.value = source->wattHoursCharged;
  injectToPayload(&currentIndex, target->payload, &item);

  item.value = source->inputVoltage;
  injectToPayload(&currentIndex, target->payload, &item);

  target->payloadLength = sizeof(target->payload)/sizeof(byte);
}

int convertToRequiredReadings(Message *source, RequiredReadings *target) {

  int currentIndex = 0;
  if (source->dataType == SK8_TEL_REQUIRED_READINGS) {

    target->ampHours = getFloatValue(&currentIndex, source->payload);
    target->rpm1 = getFloatValue(&currentIndex, source->payload);
    target->rpm2 = getFloatValue(&currentIndex, source->payload);
    target->wattHoursCharged = getFloatValue(&currentIndex, source->payload);
    target->inputVoltage = getFloatValue(&currentIndex, source->payload);
  }
  return currentIndex;
}

float getFloatValue(int *sourceIndex, byte* source) {
  FloatPayload floatPayload;
  for (int i = 0; i < 4; i++) {
    floatPayload.valueInBytes[i] = source[(*sourceIndex)++];
  }
  return floatPayload.value;
}

void injectToPayload(int *currentIndex, byte* payload, FloatPayload *item) {
  for (int i = 0; i < 4; i++) {
    payload[(*currentIndex)++] = item->valueInBytes[i];
  }
}

/**
 * Assume the first byte of the payload contains the speed value.
 */
uint8_t getSpeedValue(Message *msg) {
  return msg->payload[0];
}
