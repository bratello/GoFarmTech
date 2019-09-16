//  DHT11Value.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"DHT11Value.h"

DHT11Value::DHT11Value(uint8_t pin, bool mode, float val, float min, float max) : FloatSensorValue_t(val, min, max), _pin(pin), _tempMode(mode), _dht(pin, DHT11) { 
	
}

void DHT11Value::setup() {
	_dht.begin();
	FloatSensorValue_t::setup();
}

void	DHT11Value::doLoop() {
	auto val = (_tempMode ? _dht.readTemperature() : _dht.readHumidity());
	if(isnan(val)) {
		return;
	}
	setValue(val);
	FloatSensorValue_t::doLoop();
}