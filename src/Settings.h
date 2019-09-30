//  Settings.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef SETTINGS_H
#define SETTINGS_H
#include	<Arduino.h>
#include 	<FS.h>
#include	<functional>
#include	<list>
#include	<map>
#include	"Loggable.h"
#include	"Convertors.h"

#define INIT_PROPERTY_ONLY(property) property.setIO(this, #property)
#define INIT_PROPERTY(property, description, required) INIT_PROPERTY_ONLY(property); \
initProperty(#property, #description, required, property.getJSONReader()) 

class PropertyIO {
public:
	typedef std::list<String>	properties_t;
public:
	virtual String 			read(const String& name) = 0;
	virtual void			write(const String& name, const String& val) = 0;
	virtual bool			hasProperty(const String& name) = 0;
	virtual String 			getJSONProperty(const String& name) = 0;
	virtual	properties_t 	getAllPropertyNames() = 0;
	virtual	String			getPropertyDescription(const String& name) = 0;
	virtual bool			isRequiredProperty(const String& name) = 0;
};

template<typename T>
class Property {
protected:
	PropertyIO*		_io;
	String			_name;

	friend class Settings;

	void setIO(PropertyIO* io, const String& name) {
		_io = io;
		_name = name;
	}

	std::function<String()> getJSONReader() {
		return [this]() {
			return to_json<T>((T) *this);
		};
	}
public:

	Property() : _io(), _name() {}

	operator T () {
		return from_string<T>(_io->read(_name));
	}

	Property<T>& operator = (const T& val) {
		_io->write(_name, to_string<T>(val));
		return *this;
	}
};

class Settings : public Loggable, public PropertyIO {
	LOGGABLE(Settings)
public:
	typedef	std::function<void (void)>		event_t;
	typedef std::map<String, String>		property_map_t;
	typedef std::list<String>				file_list_t;
protected:
	bool				_enableOnChanged;
	event_t				_onChangedCallback;
	Settings();
public:
	fs::File		readFile(const String& path);
	file_list_t		readDir(const String& path);
	property_map_t	readPropertiesFile(const String& path);
	void			writePropertiesFile(const String& path, const property_map_t& table);
public:
	static  Settings* instance();

	void	onChanged(const event_t& cb) {
		if(_onChangedCallback) {
			auto prevCallback = _onChangedCallback;
			_onChangedCallback = [cb, prevCallback] () {
				prevCallback();
				cb();
			};
		} else {
			_onChangedCallback = cb;
		}
	}

	bool	enableOnChanged(bool enable) {
		auto tmp = _enableOnChanged;
		_enableOnChanged = enable;
		return tmp;
	}

	String	toJSON();

	Property<String>	deviceId;
	Property<String>	devicePwd;
	Property<String>	deviceName;
	int					deviceVersion;
	Property<String>	ssid;
	Property<String>	wifiPwd;
	Property<String>	mqttUsr;
	Property<String>	mqttPwd;
	Property<String>	mqttHost;
	Property<int>		mqttPort;
	Property<bool>		xRegDevice;
protected:
	virtual void	onInitProperties() {
		INIT_PROPERTY_ONLY(deviceId);
		INIT_PROPERTY(xRegDevice, "Register Device on ioBroker", true);
		INIT_PROPERTY(devicePwd, "Device Password", true);
		INIT_PROPERTY(deviceName, "Device Name", false);
		INIT_PROPERTY(ssid, "Network ID", true);
		INIT_PROPERTY(wifiPwd, "Network Password", true);
		INIT_PROPERTY(mqttUsr, "MQTT Server User name", false);
		INIT_PROPERTY(mqttPwd, "MQTT Server Password", false);
		INIT_PROPERTY(mqttHost, "MQTT Server Name", true);
		INIT_PROPERTY(mqttPort, "MQTT Server Port", true);
	}

	virtual void	initProperty(const String& propertyName, const String& description, bool isRequired, const std::function<String()>& jsonReader) = 0;
};

#endif