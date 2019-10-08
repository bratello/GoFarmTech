//  Settings.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"Settings.h"
#include	<map>
#include	<functional>
#include	<algorithm>
#include	<FS.h>
#include	"DataCoder.h"
#ifdef ESP32
	#include <SPIFFS.h>
#endif

#define SETTINGS_FILE	"/settings.txt"

struct PropertyMeta {
	String						description;
	bool						required;
	std::function<String()>		jsonReader;
};

class SettingsImpl : public Settings {
protected:
	typedef std::map<String, PropertyMeta>	property_meta_map_t;
	property_map_t			_properties;
	property_meta_map_t		_propertyMetaMap;

	void load();
	void save();

	virtual String 			read(const String& name);
	virtual void			write(const String& name, const String& val);
	virtual bool			hasProperty(const String& name);
	virtual String 			getJSONProperty(const String& name);
	virtual	properties_t 	getAllPropertyNames();
	virtual	String			getPropertyDescription(const String& name);
	virtual bool			isRequiredProperty(const String& name);
	virtual void			initProperty(const String& propertyName, const String& description, bool isRequired, const std::function<String()>& jsonReader);

	void					fireOnChanged();
public:
	SettingsImpl();
};


Settings* Settings::instance() {
	static SettingsImpl instance;
	return &instance;
}

Settings::Settings() : _enableOnChanged(true), _onChangedCallback([](){}) {
	int count = 0;
	LOGGER(info("Initialize SPIFFS..."))
	while(!SPIFFS.begin()) {
		::yield();
		if(!(count++ % 2)) {
			info("Initialize SPIFFS, format required");
			SPIFFS.format();
		}
		if(count > 20) {
			LOGGER(error(1, "Settings initialization failed, SPIFFS.begin() returns false"))
			return;
		}
	}
#ifdef ESP8266
	FSInfo fs_info;
	SPIFFS.info(fs_info);
	LOGGER(info("Flash total size: ") + (fs_info.totalBytes / 1024.0) + "KB")
	LOGGER(info("Flash used size: ") + (fs_info.usedBytes / 1024.0) + "KB")
#endif
}

fs::File Settings::readFile(const String& path) {
	return SPIFFS.open(path.c_str(), "r");
}

Settings::file_list_t Settings::readDir(const String& path) {
	file_list_t ret;
	LOGGER(info("readDir: ")(path));
	auto dir = SPIFFS.openDir(path);
	while(dir.next()) {
		auto fileName = dir.fileName();
		LOGGER(info(fileName));
		ret.push_back(fileName);
	}
	return ret;
}

void SettingsImpl::initProperty(const String& propertyName, const String& description, bool required, const std::function<String()>& jsonReader) {
	_propertyMetaMap[propertyName] = PropertyMeta {description, required, jsonReader};
}

SettingsImpl::SettingsImpl() : Settings() {
	onInitProperties();
}

bool		SettingsImpl::hasProperty(const String& name) {
	return (_propertyMetaMap.find(name) != _propertyMetaMap.end());
}

String 		SettingsImpl::getJSONProperty(const String& name) {
	if(!hasProperty(name)) {
		return "";
	}
	return _propertyMetaMap[name].jsonReader();
}

PropertyIO::properties_t 	SettingsImpl::getAllPropertyNames() {
	PropertyIO::properties_t keys;
	std::transform(_propertyMetaMap.begin(), _propertyMetaMap.end(), std::back_inserter(keys), [] (const property_meta_map_t::value_type& val) {
		return val.first;
	});
	return keys;
}

String		SettingsImpl::getPropertyDescription(const String& name) {
	if(!hasProperty(name)) {
		return "";
	}
	return _propertyMetaMap[name].description;
}

bool		SettingsImpl::isRequiredProperty(const String& name) {
	if(!hasProperty(name)) {
		return false;
	}
	return _propertyMetaMap[name].required;
}

void SettingsImpl::load() {
	if(!_properties.size()) {
		_properties = readPropertiesFile(SETTINGS_FILE, [] (const String& key, const String& val) {
			if(val.length() > 0 && (key == "wifiPwd" || key == "mqttPwd" || key == "mqttUsr")) {
				return DataCoder::decode(val);
			}
			return val;
		});		
	}
	return;
}

void SettingsImpl::save() {
	writePropertiesFile(SETTINGS_FILE, _properties, [] (const String& key, const String& val) {
		if(val.length() > 0 && (key == "wifiPwd" || key == "mqttPwd" || key == "mqttUsr")) {
			return DataCoder::encode(val);
		}
		return val;
	});
}

String 	SettingsImpl::read(const String& name) {
	load();
	return _properties[name];
}

void	SettingsImpl::write(const String& name, const String& val) {
	_properties[name] = val;
	save();
	fireOnChanged();
}

Settings::property_map_t	Settings::readPropertiesFile(const String& path, const property_mutator_t& predicat) {
	property_map_t props;
	LOGGER(info("Loading persistent settings: ") + path)
	auto f = SPIFFS.open(path.c_str(), "r");
	if(!f) {
		LOGGER(info("Settings file does not exists: ") + path)
		return props;
	}
	while(true) {
		::yield();
		auto s = f.readStringUntil('\n');
		if(!s.length()) {
			break;
		}
		s.trim();
		auto pos = s.indexOf('=');
		if(pos == -1) {
			LOGGER(info("wrong setting entry format: ") + s)
			continue;
		}
		auto key = s.substring(0, pos);
		auto val = s.substring(pos + 1);
		if(predicat) {
			val = predicat(key, val);
		}
		props[key] = val;
	}
	f.close();
	return props;
}

void	Settings::writePropertiesFile(const String& path, const Settings::property_map_t& table, const property_mutator_t& predicat) {
	LOGGER(info("Save settings persistently: ") + path)
	auto f = SPIFFS.open(path.c_str(), "w+");
	if(!f) {
		LOGGER(error(3, "Settings file could not be created for save: ") + path)
		return;
	}
	LOGGER(info("Settings file position: ") + f.position() + ", size: " + f.size())
	for(auto& it : table) {
		::yield();
		auto val = it.second;
		if(predicat) {
			val = predicat(it.first, val);
		}
		f.println(it.first + "=" + val);
	}
	LOGGER(info("Settings file written: ") + f.position() + "B")
	f.close();
}

void	SettingsImpl::fireOnChanged() {
	if(_enableOnChanged) {
		_onChangedCallback();
	}
}

String	Settings::toJSON() {
	auto props = getAllPropertyNames();
	props.sort([](const String& first,  const String& second) {
		return first > second;
	});
	return "{" + accumulate_chain<String>(props, [this] (const String& init, const String& propName) {
            return init + (init.length() > 0 ? "," : "") 
			+ "\""
            + propName 
            + "\":{\"value\":" + getJSONProperty(propName)
            + ",\"title\":" + getPropertyDescription(propName) 
            + ",\"required\":" + (isRequiredProperty(propName) ? "true" : "false") + "}";
        }) + "}";
}