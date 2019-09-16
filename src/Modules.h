//  Modules.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef LOGGABLE_MODULES_H
#define LOGGABLE_MODULES_H

enum class Modules {
    SYSTEM_MODULES_BEGIN = 0,
    Settings = 1,
    MQTTPlugin,
    MQTTWiFiPlugin,
    MQTTClient,
    SettingsManager,
    AccessPoint,
    NetworkManager,
    MQTTValuesStorage,

    
    VALUE_MODULES_BEGIN = 20,
    MQTTValueAbs = 21,
    RelayValue,
    SensorValue,
    LogicalValue,
    LogicalFlag,
    TimerValue,
    TimerTask,
    DHT11Value,
    SoilMoistureValue,
    TimerSlot,
    TimerTaskPipeline,

    USER_MODULES_BEGIN = 100
};

#endif