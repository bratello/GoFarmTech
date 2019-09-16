//  RelayValue.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef RELAYVALUE_H
#define RELAYVALUE_H
#include	"MQTTValue.h"

class RelayValue : public MQTTValue<bool> {
	DESCRIPTABLE(boolean, all, false)
	LOGGABLE(RelayValue)
protected:
	typedef		MQTTValue<bool>		MQTTBoolValue_t;
	uint8_t			_pin;
	bool			_inReverse;

	inline bool getPinVal(bool newVal) {
		auto val = (newVal ? HIGH : LOW);
		return (_inReverse ? (val == HIGH ? LOW : HIGH) : val);
	}
	void	onChangedRelayState(bool newVal, bool oldVal);
public:
	RelayValue(uint8_t pin = 2, bool inReverse = true, bool v = false);

	virtual		void	setup();
	virtual		void	doLoop();

	virtual bool	enable(bool val);
};

#define DEFINE_RELAY(className, PIN) \
class className : public RelayValue {\
LOGGABLE_MODULE_NAME(className) \
public: \
className(bool v = false, bool inReverse = true) : RelayValue(PIN, inReverse, v) {} \
} 

#endif