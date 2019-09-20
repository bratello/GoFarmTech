//  SystemInfo.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include	<Arduino.h>
#include	"MQTTValue.h"

class SystemInfo : public MQTTValueAbs {
    LOGGABLE(SystemInfo)
protected:
    String      fullVersion;
    String      coreVersion;
    String      sdkVersion;
    uint8_t     cpuFreqMHz;
    float_t     freeHeap;
    uint8_t     heapFragmentation;
    uint32_t    sketchSize;
    uint32_t    flashSize;

    virtual		void		doLoop();
public:
    SystemInfo();
    virtual Description getDescription();
    virtual		void	setup();
};

#endif