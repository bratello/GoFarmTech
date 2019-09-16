//  SensorValue.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef SENSOR_VALUE_H
#define SENSOR_VALUE_H
#include	"MQTTValue.h"

template<typename T>
class SensorValue : public MQTTValue<T> {
	LOGGABLE(SensorValue)
public:
	SensorValue(T val = T(), T min = T(), T max = T()) : MQTTValue<T>(val, min, max) {
		this->_skipTime = 5000;
	}

	virtual		void	setup() {
		::yield();
		Observable<T>::cleaup();
		//Subscribe only _enabled flag
		MQTTValue<T>::setupEnabled();
		this->_minVal = this->loadMinValue(this->_minVal);
		this->_maxVal = this->loadMaxValue(this->_maxVal);
		this->_skipTime = this->loadSkipTime(this->_skipTime);
		//Sensor value can't be changed from MQTT server, only limits
		this->_client->subscribe("minValue", make_subscriber<T>([this] (const T& val) {
			this->_minVal = this->saveMinValue(val);
			this->_client->publish("minValue", this->_minVal);
		}));
		this->_client->subscribe("maxValue", make_subscriber<T>([this] (const T& val) {
			this->_maxVal = this->saveMaxValue(val);
			this->_client->publish("maxValue", this->_maxVal);
		}));
		this->_client->subscribe("skipTime", make_subscriber<time_t>([this] (const time_t& val) {
			this->_skipTime = this->saveSkipTime(val);
			this->_client->publish("skipTime", this->_skipTime);
		}));
		LOGGER(this->info("Setup done"))
	}

	virtual Description getDescription() {
		Description meta;
		meta.setName(getModuleName());
		meta.setType(Description::Type::number);
		meta.setAccess(Description::Access::read);
		meta.setDefaultValue(T());
		meta.setValue(this->getValue());

		Description min = meta;
		min.setName(String("minValue"));
		min.setAccess(Description::Access::write);
		min.setValue(this->_minVal);
		min.setDefaultValue(this->_minMaxOff);

		Description max = meta;
		max.setName(String("maxValue"));
		max.setAccess(Description::Access::write);
		max.setValue(this->_maxVal);
		max.setDefaultValue(this->_minMaxOff);

		Description skip = meta;
		skip.setName(String("skipTime"));
		skip.setAccess(Description::Access::write);
		skip.setValue(this->_skipTime);
		skip.setDefaultValue(this->_skipTime);

		Description enabledMeta;
		enabledMeta.setName("enabled");
		enabledMeta.setType(Description::Type::boolean);
		enabledMeta.setAccess(Description::Access::all);
		enabledMeta.setDefaultValue(true);
		enabledMeta.setValue(this->isEnabled());
		
		meta.addAttribute(enabledMeta);
		meta.addAttribute(min);
		meta.addAttribute(max);
		meta.addAttribute(skip);
		return meta;
	}
};

#endif