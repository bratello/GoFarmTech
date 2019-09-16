//  RelayValue.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"RelayValue.h"

using namespace std::placeholders;

RelayValue::RelayValue(uint8_t pin, bool inReverse, bool v) : MQTTBoolValue_t(v), _pin(pin), _inReverse(inReverse) {
	
}

void	RelayValue::onChangedRelayState(bool newVal, bool oldVal) {
	LOGGER(info("Relay value changed: ") + newVal)
	auto val = getPinVal(newVal);
	digitalWrite(_pin, val);
}

bool	RelayValue::enable(bool val) {
	auto ret = MQTTBoolValue_t::enable(val);
	if(!val) {
		//Switch off relay
		setValue(false);
	}
	return ret;
}

void	RelayValue::setup() {
	MQTTBoolValue_t::setup();
	//Setup PIN mode
	pinMode(_pin, OUTPUT);
	//Setup pin initial value
	digitalWrite(_pin, getPinVal(false));
	auto v = getValue();
	onChangedRelayState(v, !v);
	this->onChanged(std::bind(&RelayValue::onChangedRelayState, this, _1, _2));
}

void	RelayValue::doLoop() {
	MQTTBoolValue_t::doLoop();
}