//  MQTTValue.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef MQTTVALUE_H
#define MQTTVALUE_H
#include	<Arduino.h>
#include	<functional>
#include	"Observable.h"
#include	"Runnable.h"
#include	"Loggable.h"
#include	"Descriptable.h"
#include	"MQTTClient.h"

class MQTTValueAbs : public Runnable, public Descriptable, public Loggable {
	LOGGABLE(MQTTValueAbs)
protected:
	MQTTClient*		_client;

	virtual String		getAttributeFullName(const String& attrName);
	String 		loadAttributeStr(const String& attrName);
	bool		isAttributePersisted(const String& attrName);
	String		saveAttributeStr(const String& attrName, const String& val);

	template<typename T>
	T 		loadAttribute(const String& attrName, const T& defVal = T()) {
		::yield();
		if(isAttributePersisted(attrName)) {
			return from_string<T>(loadAttributeStr(attrName));
		} else {
			return saveAttribute(attrName, defVal);
		}
	}

	template<typename T>
	T	saveAttribute(const String& attrName, const T& val) {
		::yield();
		saveAttributeStr(attrName, to_string(val));
		return val;
	}

	template<typename T>
	T  loadMinValue(const T& val) {
		return loadAttribute<T>("minVal", val);
	}

	template<typename T>
	T  saveMinValue(const T& val) {
		return saveAttribute<T>("minVal", val);
	}

	template<typename T>
	T  loadMaxValue(const T& val) {
		return loadAttribute<T>("maxVal", val);
	}

	template<typename T>
	T  saveMaxValue(const T& val) {
		return saveAttribute<T>("maxVal", val);
	}

	time_t  loadSkipTime(const time_t& val) {
		return loadAttribute<time_t>("skipTime", val);
	}

	time_t  saveSkipTime(const time_t& val) {
		return saveAttribute<time_t>("skipTime", val);
	}
public:
	MQTTValueAbs();
	virtual ~MQTTValueAbs();
	virtual void	setClient(MQTTClient* obj);
public:
	virtual void	pinMode(uint8_t pin, uint8_t mode);
	virtual void	digitalWrite(uint8_t pin, uint8_t val);
	virtual int		digitalRead(uint8_t pin);
	virtual int		analogRead(uint8_t pin);
};

template<typename T>
class MQTTValue : public Observable<T>, public MQTTValueAbs {
protected:
	void setupEnabled() {
		this->_enabled = loadAttribute("enabled", this->_enabled);
		this->_client->subscribe("enabled", make_subscriber<bool>([this] (const bool& val) {
			enable(saveAttribute("enabled", val));
		}));
		enable(this->_enabled);
	}

	void setupValue() {
		_client->subscribe("", make_subscriber<T>([this] (const T& val) {
			//LOGGER(
			//	info("Value changed from server: ")(val)
			//	(", oldVal: ")(this->_val)
			//	(", isEnabled: ")(this->isEnabled())
			//	(", onChangedEvents: ")(this->_onChangedEvents.size())
			//)
			this->setValue(val);
		}));
	}
public:
	MQTTValue(T v = T(), T min = T(), T max = T()) : Observable<T>(v, min, max), MQTTValueAbs() {
		::yield();
	}

	virtual		void	setup() {
		::yield();		
		setupValue();
		setupEnabled();
		LOGGER(info("Setup done"))
	}

	virtual		void	doLoop() {
		::yield();
		if(this->_changed) {
			_client->publish("", this->getValue());
			this->_changed = false;
		}
	}
	virtual bool	enable(bool val) {
		if(this->isEnabled() != val)
			this->_client->publish("enabled", val);
		return Observable<T>::enable(val);
	}
};

#endif
