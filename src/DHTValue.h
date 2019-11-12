//  DHT11Value.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef DHTVALUE_H
#define DHTVALUE_H
#include	"SensorValue.h"
#include 	<DHT.h>

class DHTValue : public SensorValue<float> {
	LOGGABLE(DHTValue)
protected:
	uint8_t		_pin = 0;
	bool		_tempMode = false;

	virtual		void	doLoop();
public:
	typedef 	SensorValue<float>	FloatSensorValue_t;
	DHTValue(uint8_t pin = 0, bool mode = false, uint8_t model = DHT11, float val = float(), float min = float(), float max = float());
	virtual		void	setup();
};

#define DEFINE_DHT11(className, pin, isTempMode, min, max) class className : public DHTValue { \
LOGGABLE_MODULE_NAME(className) \
public: \
className(float val = float()) : DHTValue(pin, isTempMode, DHT11, val, min, max) {} \
}

#define DEFINE_DHT22(className, pin, isTempMode, min, max) class className : public DHTValue { \
LOGGABLE_MODULE_NAME(className) \
public: \
className(float val = float()) : DHTValue(pin, isTempMode, DHT22, val, min, max) {} \
}

#endif