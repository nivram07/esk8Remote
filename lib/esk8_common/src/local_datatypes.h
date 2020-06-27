#ifndef LOCAL_DATATYPES_H_
#define LOCAL_DATATYPES_H_

// Added by AC to store measured values
struct bldcMeasure {
	float tempFetFiltered;
	float tempFetFiltered2;
	float tempMotorFiltered;
	float tempMotorFiltered2;
	float avgMotorCurrent;
	float avgMotorCurrent2;
	float avgInputCurrent;
	float avgId;
	float avgId2;
	float avgIq;
	float avgIq2;
	float dutyNow;
	float dutyNow2;
	float rpm;
	float rpm2;
	float inpVoltage;
	float ampHours;
	float ampHoursCharged;
	float wattHours;
	float wattHoursCharged;
	int32_t tachometer;
	int32_t tachometer2;
	int32_t tachometerAbs;
	int32_t tachometerAbs2;
	uint8_t faultCode;
};
//Define remote Package

struct remotePackage {

	int		valXJoy;
	int		valYJoy;
	boolean	valUpperButton;
	boolean	valLowerButton;

};


typedef enum DATA_TYPE {
    SK8_MESSAGE,
    SK8_SPEED,
    SK8_TEL_AMP_HR_CHARGED,
    SK8_TEL_RPM1,
    SK8_TEL_RPM2,
    SK8_TEL_WATT_HRS_CHARGED,
    SK8_TEL_AVG_INPUT_CURRENT,
    SK8_TEL_REQUIRED_READINGS
} DATA_TYPE;


typedef struct RequiredReadings {
  float ampHours;
  float rpm1;
  float rpm2;
  float wattHoursCharged;
  float inputVoltage;
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


#endif
