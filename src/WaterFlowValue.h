//  WaterFlowValue.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef WATERFLOWVALUE_H
#define WATERFLOWVALUE_H
#include	"SensorValue.h"

class WaterFlowValue : public SensorValue<ulong> {
	LOGGABLE(WaterFlowValue)
public:
    typedef     std::function<void (ulong)>     value_reached_event_t;
protected:
	typedef 	SensorValue<ulong>	WaterFlowValueBase;
    struct      ValueReachedEventInfo {
        ulong                   _started;
        ulong                   _required;
        value_reached_event_t   _event;
    };

    typedef     std::list<ValueReachedEventInfo>    value_reached_events_t;

	uint8_t		        _pin = 0;
    time_t	            _prevTime = 0;
    value_reached_events_t
                        _value_reached_events;

	virtual		void	doLoop();
    virtual     ulong   getWaterUsage() = 0;
    virtual     ulong   calcWaterUsage(ulong counter, float flowFactor, bool inMilliLiters);
    virtual     void    setupValueReachedEvents();
    virtual     void    setupPinInterrupt(void (*)(void));
public:
    WaterFlowValue(uint8_t pin, ulong val = ulong(), ulong min = ulong(), ulong max = ulong());

    virtual		time_t	loop(time_t time = 0);
    virtual		void	setup();
    virtual     void    onValueFromNowReached(ulong val, const value_reached_event_t& callback);
};

#define DEFINE_WATERFLOW_SENSOR(className, pin, min, max, flowFactor, inMilliLiters) class className : public WaterFlowValue { \
LOGGABLE_MODULE_NAME(className) \
protected:\
    static ulong flowCounter(ulong inc = 1) {\
        static volatile ulong counter = 0;\
        if(inc > 0) { \
            counter += inc;\
            return counter;\
        } else {\
            auto ret = counter;\
            counter = 0;\
            return ret;\
        }\
    }\
    ICACHE_RAM_ATTR static void flowInterrupter() {\
        className::flowCounter();\
    }\
    virtual     ulong   getWaterUsage() {\
        return calcWaterUsage(className::flowCounter(0), flowFactor, inMilliLiters);\
    }\
public: \
    className(ulong val = ulong()) : WaterFlowValue(pin, val, min, max) {} \
    virtual		void	setup() {\
        WaterFlowValue::setup(); \
        setupPinInterrupt(className::flowInterrupter);\
    }\
}

#define DEFINE_WATERFLOW(className, pin, flowFactor) DEFINE_WATERFLOW_SENSOR(className, pin, 0, 0, flowFactor, false)

#endif