//  MQTTWiFiPlugin.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef MQTTWIFIPLUGIN_H
#define MQTTWIFIPLUGIN_H
#if defined(ESP8266)
	#include 	<ESP8266WiFi.h>
#else
    #include <WiFi.h>
#endif
#include	"MQTTPlugin.h"

class MQTTWiFiPlugin : public MQTTPlugin {
	LOGGABLE(MQTTWiFiPlugin)
protected:
	WiFiClient		_espClient;
public:
	MQTTWiFiPlugin();

//Runnable implementation
public:
	virtual		void	setup();
};
#endif