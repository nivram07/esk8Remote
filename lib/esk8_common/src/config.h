#ifndef _CONFIG_H_
#define _CONFIG_H_

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#ifdef ARDUINO_AVR_NANO

//Pin definition

//nRF24:
/*nRF24	>	Ardunio nano
------------------------
VCC		>	3.3 V
GND		>	GND
MOSI	>	11
MISO	>	12
SCK		>	13
IRQ		>	not connected*/
//#define CEPIN	9
//#define CSPIN	10
/*VESC UART
VESC		Arduino Nano
VCC		>	5V			(black wire)
GND		>	GND			(white wire)
TX		>	RX			(orange wire)
RX		>	TX			(green wire)
*/
//Definition of Serial ports
#define SERIALIO Serial
#define DEBUGSERIAL Serial
#endif

#define RECEIVER_ADDRESS_INDEX 0
#define TRANSMITTER_ADDRESS_INDEX 1

extern byte addresses[][6];

#endif
