//  MQTTPlugin.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef MQTTPLUGIN_H
#define MQTTPLUGIN_H
#include	<Arduino.h>

#include	<list>
#include	<algorithm>

#include	"Runnable.h"
#include	"Loggable.h"
#include	"Descriptable.h"
#include	"MQTTValue.h"
#include	"TimerValue.h"
#include	"Settings.h"
#include 	"MQTTClient.h"
#include	"SettingsManager.h"
#include	"SystemInfo.h"


class MQTTPlugin : public Runnable, public Loggable, public Descriptable {
	LOGGABLE(MQTTPlugin)
protected:
	typedef		std::list<MQTTValueAbs*>	mqtt_values_t;
	MQTTClient*			_client;
	mqtt_values_t		_values;

	TimerValue			_timer;
	SettingsManager		_settingsManager;
	SystemInfo			_sysInfo;
	uint16_t			_loopValueIndex;
//Runnable implementation
protected:
	virtual		void	doLoop();
	virtual		void	setupValues() = 0;
	virtual		void	setupValue(MQTTValueAbs& val);
	virtual		void	setupTimer(Timer timer);
	virtual		void	setupLogs();
	virtual		void	setupInfra();
	virtual		void	setupDeviceLogic();
	virtual		void	loopValues(time_t ticks);
	virtual		void	setupSysTasks(Timer timer) = 0;
public:
	virtual		void	setup();

	virtual		Description getDescription();
public:
	MQTTPlugin();
};

#endif