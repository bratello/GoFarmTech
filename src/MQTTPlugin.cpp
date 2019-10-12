//  MQTTPlugin.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"MQTTPlugin.h"

#define VERIFY_MIN_DELAY(exp) { auto skipTime = exp; if(minSkipTime == -1 || minSkipTime > skipTime) minSkipTime = skipTime; }

MQTTPlugin::MQTTPlugin() : _client(0) {
	_skipTime = 0;
}

void MQTTPlugin::doLoop() {
	if(!_client) {
		LOGGER(error(1, "Initialize plugin first!"))
		return;
	}
	time_t minSkipTime = -1;
	VERIFY_MIN_DELAY(_client->loop(_lastTime));
	yield();
	logMemUsage("is still available");
	VERIFY_MIN_DELAY(_settingsManager.loop(_lastTime));
	yield();
	VERIFY_MIN_DELAY(loopValues(_lastTime));
	if(minSkipTime > 0)
		delay(minSkipTime < 1000 ? minSkipTime : 1000);
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

time_t 	MQTTPlugin::loopValues(time_t ticks) {
	time_t minSkipTime = -1;
	for(auto val : _values) {
		::yield();
		VERIFY_MIN_DELAY(val->loop(ticks));
	}
	return minSkipTime;
}