//  MQTTWiFiPlugin.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"MQTTWiFiPlugin.h"
#include 	"NetworkManager.h"

MQTTWiFiPlugin::MQTTWiFiPlugin() : MQTTPlugin() {}

void	MQTTWiFiPlugin::setup() {
	setupLogs();
	_client = MQTTClient::instance(	_espClient);
	delay(10);

	netManager.connect();

	Settings::instance()->onChanged([]() {
		netManager.connect();
	});
	MQTTPlugin::setup();
}