//  SoilMoistureValue.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef SOILMOISTUREVALUE_H
#define SOILMOISTUREVALUE_H
#include	"SensorValue.h"

////////////////////////////////////////////////////////////////////////////////////////////
// See example: 
// http://www.instructables.com/id/ESP8266-Soil-Moisture-Sensor-With-Arduino-IDE/
// https://maker.pro/arduino/projects/arduino-soil-moisture-sensor
class SoilMoistureValue : public SensorValue<float> {
	LOGGABLE(SoilMoistureValue)
protected:
	typedef 	SensorValue<float>	FloatSensorValue_t;

	uint8_t		_pin = 0;
	virtual		void	doLoop();
public:
	SoilMoistureValue(uint8_t pin = A0, float val = float(), float min = float(), float max = float());
	virtual		void	setup();
};

#define DEFINE_SOILSENSOR(className, pin, min, max) class className : public SoilMoistureValue { \
LOGGABLE_MODULE_NAME(className) \
public: \
className(float val = float()) : SoilMoistureValue(pin, val, min, max) {} \
}

#endif