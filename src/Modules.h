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

    TIMER_MODULES_BEGIN = 20,
    TimerValue = 21,
    TimerTask,
    TimerSlot,
    TimerTaskPipeline,

    VALUE_MODULES_BEGIN = 40,
    MQTTValueAbs = 41,
    RelayValue,
    SensorValue,
    LogicalValue,
    LogicalFlag,
    SystemInfo,
    DHT11Value,
    SoilMoistureValue,
    WaterFlowValue,
    

    USER_MODULES_BEGIN = 100
};

#endif