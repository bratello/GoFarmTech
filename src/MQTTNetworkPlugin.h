//  MQTTNetworkPlugin.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef MQTTWIFIPLUGIN_H
#define MQTTWIFIPLUGIN_H

#include	"MQTTPlugin.h"
#include 	"NetworkManager.h"

class MQTTNetworkPlugin : public MQTTPlugin {
	LOGGABLE(MQTTNetworkPlugin)
protected:
	virtual		void	setupSysTasks(Timer timer);
	virtual		void	onConnectionStatusChanged(NetworkConnectionStatus status);
public:
	MQTTNetworkPlugin();

//Runnable implementation
public:
	virtual		void	setup();
};
#endif