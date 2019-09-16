//  SettingsManager.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include	<Arduino.h>
#include    <memory>
#include	"Runnable.h"
#include	"Loggable.h"
#include    "Descriptable.h"
#include    "MQTTClient.h"


#ifndef SETTINGS_PIN
    #define SETTINGS_PIN D3
#endif
#ifndef SETTINGS_LED
    #define SETTINGS_LED 2
#endif

class SettingsManager : public Runnable, public Loggable {
    LOGGABLE(SettingsManager)
protected:
    typedef 
        std::unique_ptr<Runnable>  
                            RunnablePtr;

    uint8_t                 _modeLeadPin;
    uint8_t                 _switchPin;
    bool                    _apMode;
    RunnablePtr             _accessPoint;
    Descriptable*           _deviceDescriptor;
    MQTTClientTransmitter*  _client;

    virtual		void	doLoop();
    void                handle();
public:
    SettingsManager(uint8_t leadPin = SETTINGS_LED, uint8_t switchPin = SETTINGS_PIN);

    virtual		void	setup();
    void        setDeviceDescriptor(Descriptable* device);
    void        setClient(MQTTClientTransmitter* client);
};

#endif