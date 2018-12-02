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
