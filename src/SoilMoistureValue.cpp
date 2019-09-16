//  SoilMoistureValue.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"SoilMoistureValue.h"

SoilMoistureValue::SoilMoistureValue(uint8_t pin, float val, float min, float max) : FloatSensorValue_t(val, min, max), _pin(pin) {}

void	SoilMoistureValue::setup() {
	FloatSensorValue_t::setup();
}

void	SoilMoistureValue::doLoop() {
	auto analogValue = analogRead(_pin);
	// convert the analog signal to voltage
	// the ESP2866 A0 reads between 0 and ~3 volts, producing a corresponding value
	// between 0 and 1024. The equation below will convert the value to a voltage value.

	setValue(map(analogValue,0, 1024, 100, 0));
	FloatSensorValue_t::doLoop();
}