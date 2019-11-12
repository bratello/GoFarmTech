//  DHT11Value.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"DHTValue.h"

#define	DHT_MAX 50
typedef	std::unique_ptr<DHT>		DHTPtr;
static	DHTPtr	__dhtMap[DHT_MAX] = {};
static	bool	__dhtInitializer[DHT_MAX] = {};

DHTValue::DHTValue(uint8_t pin, bool mode, uint8_t model, float val, float min, float max) : FloatSensorValue_t(val, min, max), _pin(pin), _tempMode(mode) { 
	if(!__dhtMap[pin]) {
		__dhtMap[pin] = DHTPtr(new DHT(pin, model));
	}
}

void DHTValue::setup() {
	if(!__dhtInitializer[_pin]) {
		__dhtMap[_pin]->begin();
		__dhtInitializer[_pin] = true;
	}
	FloatSensorValue_t::setup();
}

void	DHTValue::doLoop() {
	auto val = (_tempMode ? __dhtMap[_pin]->readTemperature() : __dhtMap[_pin]->readHumidity());
	if(isnan(val)) {
		return;
	}
	setValue(val);
	FloatSensorValue_t::doLoop();
}