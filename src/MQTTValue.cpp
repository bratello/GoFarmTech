//  MQTTValue.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"MQTTValue.h"
#include	"Settings.h"

class MQTTValuesStorageProxy {
protected:
    String* val;
    String key;
    MQTTValuesStorageProxy& make_proxy(const String& k, String* v) {
        key = k;
        val = v;
        return *this;
    }

	virtual void onStorageChanged(const String& key) = 0;
public:
    
    operator String() const {
        return *val;
    }
    
    MQTTValuesStorageProxy& operator=(const String& v) {
        *val = v;
		onStorageChanged(key);
        return *this;
    }
};

class MQTTValuesStorage : public Settings::property_map_t, public Loggable, protected MQTTValuesStorageProxy {
	LOGGABLE(MQTTValuesStorage)
protected:
	String valueSettingsFile = "/values.txt";
	MQTTValuesStorage() : Settings::property_map_t() {
		auto map = Settings::instance()->readPropertiesFile(valueSettingsFile);
		this->insert(map.begin(), map.end());
	}

	virtual void onStorageChanged(const String& key) {
		LOGGER(info("onStorageChanged: ")(key))
		Settings::instance()->writePropertiesFile(valueSettingsFile, *this);
	}
public:
	static MQTTValuesStorage& storage() {
		static MQTTValuesStorage instance;
		return instance;
	}

	MQTTValuesStorageProxy& operator[]( const String& key ) {
		return make_proxy(key, &Settings::property_map_t::operator[](key));
	}
};

inline MQTTValuesStorage& values_storage() {
	return MQTTValuesStorage::storage();
}

MQTTValueAbs::MQTTValueAbs() : _client(NULL) {
	::yield();
	this->_skipTime = 1000;
}

MQTTValueAbs::~MQTTValueAbs() {}

void	MQTTValueAbs::setClient(MQTTClient* obj) {
	_client = obj->getChildClient(getModuleName());
}

void MQTTValueAbs::pinMode(uint8_t pin, uint8_t mode) {
	::pinMode(pin, mode);
}

void MQTTValueAbs::digitalWrite(uint8_t pin, uint8_t val) {
	::digitalWrite(pin, val);
}

int MQTTValueAbs::digitalRead(uint8_t pin) {
	return ::digitalRead(pin);
}

int	MQTTValueAbs::analogRead(uint8_t pin) {
	return ::analogRead(pin);
}

String	MQTTValueAbs::getAttributeFullName(const String& attrName) {
	return getModuleName() + (attrName.length() > 0 ? ":" + attrName : "");
}

bool   MQTTValueAbs::isAttributePersisted(const String& attrName) {
	return values_storage().find(getAttributeFullName(attrName)) != values_storage().end();
}

String MQTTValueAbs::loadAttributeStr(const String& attrName) {
	auto fullName = getAttributeFullName(attrName);
	if(values_storage().find(fullName) != values_storage().end()) {
		return values_storage()[fullName];
	}
	return "";
}

String MQTTValueAbs::saveAttributeStr(const String& attrName, const String& val) {
	auto fullName = getAttributeFullName(attrName);
	values_storage()[fullName] = val;
	return val;
}