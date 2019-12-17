//  MQTTNetworkPlugin.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"MQTTNetworkPlugin.h"

MQTTNetworkPlugin::MQTTNetworkPlugin() : MQTTPlugin() {}

void	MQTTNetworkPlugin::setup() {
	setupLogs();
	_client = MQTTClient::instance(	netManager.getClient() );
	delay(10);

	netManager.connect();

	Settings::instance()->onChanged([]() {
		netManager.connect();
	});
	MQTTPlugin::setup();
}

void	MQTTNetworkPlugin::onConnectionStatusChanged(NetworkConnectionStatus status) {
	if(status == NetworkConnectionStatus::DISCONNECTED)
		LOGGER(info("Device network disconnected"))
	else if(status == NetworkConnectionStatus::CONNECTING)
		LOGGER(info("Restart network connection"))
	else
		LOGGER(info("Device network connected"))
}

void	MQTTNetworkPlugin::setupSysTasks(Timer timer) {
	timer.on("sysNetAdmin",{ 
		{0, TIME_INTERVAL::DAY, 10, TIME_INTERVAL::MINUTE * 5, TimerSlot::ALL_DAYS, TimerSlot::ALL_MONTHS} 
	}, [this] () {
		if(!netManager.isConnected() && !netManager.isSetupConnected()) {
			onConnectionStatusChanged(NetworkConnectionStatus::DISCONNECTED);
			netManager.disconnect();
		}
	}, [this] () {
		if(!netManager.isConnected() && !netManager.isSetupConnected()) {
			onConnectionStatusChanged(NetworkConnectionStatus::CONNECTING);
			if(!netManager.connect()) {
				onConnectionStatusChanged(NetworkConnectionStatus::DISCONNECTED);
			} else {
				onConnectionStatusChanged(NetworkConnectionStatus::CONNECTED);
			}
		}
	});
}