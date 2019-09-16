//  MQTTPlugin.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"MQTTPlugin.h"
#include	"NetworkManager.h"

MQTTPlugin::MQTTPlugin() : _client(0) {
	_skipTime = 0;
}

void MQTTPlugin::doLoop() {
	if(!_client) {
		LOGGER(error(1, "Initialize plugin first!"))
		return;
	}
	_client->loop(_lastTime);
	yield();
	logMemUsage("is still available");
	_settingsManager.loop(_lastTime);
	yield();
	loopValues(_lastTime);
	if(!(millis() - _lastTime))
		delay(25);
}

void MQTTPlugin::setup() {
	if(!_client) {
		LOGGER(error(2, "Initialize plugin first!"))
		return;
	}
	_client->setup();
	LOGGER(info("Plugin setup"));
	_values.clear();
	setupTimer(_timer);
	setupValue(_timer);
	setupValues();
	setupInfra();
	setupDeviceLogic();
	_client->setDeviceDescriptor(this);
	_settingsManager.setDeviceDescriptor(this);
	_settingsManager.setClient(_client);
	_settingsManager.setup();
}

Description MQTTPlugin::getDescription() {
	Description meta;
	meta.setName(getModuleName());
	meta.setDefaultValue(0);
	meta.setValue(0);
	meta.setType(Description::Type::object);
	meta.setAccess(Description::Access::read);
	meta.setVersion(Settings::instance()->deviceVersion);
	for(mqtt_values_t::iterator it = _values.begin(); it != _values.end(); it++) {
		Description item = (*it)->getDescription();
		meta.addChild(item);
	}
	Description lastErrorMeta;
	lastErrorMeta.setName("LastError");
	lastErrorMeta.setDefaultValue(String(""));
	lastErrorMeta.setValue(String(""));
	lastErrorMeta.setAccess(Description::Access::read);
	lastErrorMeta.setType(Description::Type::string);
	meta.addAttribute(lastErrorMeta);
	return meta;
}

void	MQTTPlugin::setupValue(MQTTValueAbs& val) {
	::yield();
	val.setClient(_client);
	val.setup();
	_values.push_back(&val);
}

void	MQTTPlugin::setupTimer(Timer timer) {
	//Do nothing
}

void	MQTTPlugin::setupLogs() {
	::yield();
	addLogOutput([] (const String& str) {
		Serial.println(str);
		Serial.flush();
	});
	Serial.println("");
	LOGGER(info("setupLogs done"))
	logMemUsage("at the beginning");
}

void 	MQTTPlugin::setupInfra() {
	//Do nothing
}

void	MQTTPlugin::setupDeviceLogic() {
	//Do nothing
}

void 	MQTTPlugin::loopValues(time_t ticks) {
	for(auto val : _values) {
		::yield();
		val->loop(ticks);
	}
	publishLastError();
}

void	MQTTPlugin::publishLastError() {
	if(isLastErrorAvailable()) {
		_client->publish("LastError", fetchLastError());
	}
}