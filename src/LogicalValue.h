//  LogicalValue.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef LOGICAL_VALUE_H
#define LOGICAL_VALUE_H
#include	"MQTTValue.h"

template<typename T>
class LogicalValue : public MQTTValue<T> {
    LOGGABLE(LogicalValue)
public:
    LogicalValue(T val = T(), T min = T(), T max = T()) : MQTTValue<T>(val, min, max) {
		this->_skipTime = 5000;
	}

    virtual		void	setup() {
		yield();
		Observable<T>::cleaup();
		MQTTValue<T>::setupEnabled();
		this->_minVal = this->loadMinValue(this->_minVal);
		this->_maxVal = this->loadMaxValue(this->_maxVal);
		this->_skipTime = this->loadSkipTime(this->_skipTime);
        this->_val = this->loadAttribute("", this->_val);
        this->_client->subscribe("", make_subscriber<T>([this] (const T& val) {
			this->setValue(this->saveAttribute("", val));
		}));
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
		meta.setAccess(Description::Access::all);
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
		enabledMeta.setAccess(Description::Access::write);
		enabledMeta.setDefaultValue(true);
		enabledMeta.setValue(this->isEnabled());
		
		meta.addAttribute(enabledMeta);
		meta.addAttribute(min);
		meta.addAttribute(max);
		meta.addAttribute(skip);
		return meta;
	}
};

class LogicalFlag : public MQTTValue<bool> {
    LOGGABLE(LogicalFlag)
public:
    LogicalFlag(bool val = bool()) : MQTTValue<bool>(val) {
		this->_skipTime = 1000;
	}

    virtual		void	setup() {
		yield();
		Observable<bool>::cleaup();
		MQTTValue<bool>::setupEnabled();
        this->_val = this->loadAttribute("", this->_val);
		this->_skipTime = this->loadSkipTime(this->_skipTime);
        this->_client->subscribe("", make_subscriber<bool>([this] (const bool& val) {
			this->setValue(this->saveAttribute("", val));
		}));
		this->_client->subscribe("skipTime", make_subscriber<time_t>([this] (const time_t& val) {
			this->_skipTime = this->saveSkipTime(val);
			this->_client->publish("skipTime", this->_skipTime);
		}));
    }

    virtual Description getDescription() {
		Description meta;
		meta.setName(getModuleName());
		meta.setType(Description::Type::boolean);
		meta.setAccess(Description::Access::all);
		meta.setDefaultValue(bool());
		meta.setValue(this->getValue());

		Description skip = meta;
		skip.setName(String("skipTime"));
		skip.setType(Description::Type::number);
		skip.setAccess(Description::Access::write);
		skip.setValue(this->_skipTime);
		skip.setDefaultValue(this->_skipTime);

		Description enabledMeta;
		enabledMeta.setName("enabled");
		enabledMeta.setType(Description::Type::boolean);
		enabledMeta.setAccess(Description::Access::write);
		enabledMeta.setDefaultValue(true);
		enabledMeta.setValue(this->isEnabled());
		
		meta.addAttribute(enabledMeta);
		meta.addAttribute(skip);
		return meta;
	}
};

#define DEFINE_LOGICAL_VALUE(className, type, initialValue, min, max) class className : public LogicalValue<type> { \
LOGGABLE_MODULE_NAME(className) \
public: \
className(type val = initialValue) : LogicalValue<type>(val, min, max) {} \
}

#define DEFINE_LOGICAL_FLAG(className, initialValue) class className : public LogicalFlag { \
LOGGABLE_MODULE_NAME(className) \
public: \
className(bool val = initialValue) : LogicalFlag(val) {} \
}

#endif