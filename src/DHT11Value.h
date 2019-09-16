//  DHT11Value.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef DHT11VALUE_H
#define DHT11VALUE_H
#include 	<DHT.h>
#include	"SensorValue.h"

class DHT11Value : public SensorValue<float> {
	LOGGABLE(DHT11Value)
protected:
	uint8_t		_pin = 0;
	bool		_tempMode = false;
	DHT 		_dht;

	virtual		void	doLoop();
public:
	typedef 	SensorValue<float>	FloatSensorValue_t;
	DHT11Value(uint8_t pin = 0, bool mode = false, float val = float(), float min = float(), float max = float());
	virtual		void	setup();
};

#define DEFINE_DHT11(className, pin, isTempMode, min, max) class className : public DHT11Value { \
LOGGABLE_MODULE_NAME(className) \
public: \
className(float val = float()) : DHT11Value(pin, isTempMode, val, min, max) {} \
}

#endif