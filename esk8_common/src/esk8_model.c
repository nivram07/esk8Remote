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


/**
 * Assume the first byte of the payload contains the speed value.
 */
uint8_t getSpeedValue(Message *msg) {
  return msg->payload[0];
}
