//  MQTTPlugin.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"MQTTPlugin.h"

MQTTPlugin::MQTTPlugin() : _client(0), _loopValueIndex(0) {
	_skipTime = 0;
}

void MQTTPlugin::doLoop() {
	if(!_client) {
		LOGGER(error(1, "Initialize plugin first!"))
		return;
	}
	if(_client->nextLoop() <= _lastTime)
		_client->loop(_lastTime);
	yield();
	logMemUsage("is still available");
	if(_settingsManager.nextLoop() <= _lastTime)
		_settingsManager.loop(_lastTime);
	yield();
	loopValues(_lastTime);
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
	setupSysTasks(_timer);
	setupValue(_timer);
	setupValue(_sysInfo);
	setupValues();
	setupInfra();
	setupDeviceLogic();
	_client->setDeviceDescriptor(this);
	_settingsManager.setDeviceDescriptor(this);
	_settingsManager.setClient(_client);
	_settingsManager.setup();
	_loopValueIndex = 0;
}

Description MQTTPlugin::getDescription() {
	Description meta;
	String deviceName = Settings::instance()->deviceName;
	if(!deviceName.length()) {
		deviceName = getModuleName();
	}
	meta.setName(deviceName);
	meta.setDefaultValue(0);
	meta.setValue(0);
	meta.setType(Description::Type::object);
	meta.setAccess(Description::Access::read);
	meta.setVersion(Settings::instance()->deviceVersion);
	for(mqtt_values_t::iterator it = _values.begin(); it != _values.end(); it++) {
		Description item = (*it)->getDescription();
		meta.addChild(item);
	}
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
	if(_loopValueIndex >= _values.size()) {
		_loopValueIndex = 0;
	}
	while(_loopValueIndex < _values.size()) {
		auto it = std::next(_values.begin(), _loopValueIndex++);
		if(it != _values.end() && (*it)->nextLoop() <= ticks) {
			(*it)->loop(ticks);
			break;
		}
	}
	if(_loopValueIndex >= _values.size()) {
		_loopValueIndex = 0;
	}
	return;
}