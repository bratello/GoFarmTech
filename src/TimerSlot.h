//  TimerSlot.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef TIMER_SLOT_H
#define TIMER_SLOT_H
#include	<Arduino.h>
#include    <Arduino_JSON.h>
#include	<TimeLib.h>
#include 	<list>
#include    "Loggable.h"

enum TIME_INTERVAL {
    MINUTE = 60,
    HOUR = 3600,
    MIDDAY = 43200,
    DAY = 86400
};

typedef long seconds_t;
inline seconds_t make_seconds(const TimeElements& now) {
    return now.Hour * TIME_INTERVAL::HOUR + now.Minute * TIME_INTERVAL::MINUTE + now.Second;
}

class TimerSlot : public Loggable {
    LOGGABLE(TimerSlot)
public:
    enum WEEK_DAYS_VALUE {
        SUN = 1,
        MON = 2,
        TUE = 4,
        WEN = 8,
        THR = 16,
        FRI = 32,
        SAT = 64,
        ALL_DAYS = 127
    };

    enum MONTHS_VALUE {
        JAN = 1,
        FEB = 2,
        MAR = 4,
        APR = 8,
        MAY = 16,
        JUN = 32,
        JUL = 64,
        AUG = 128,
        SEP = 256,
        OCTB = 512,
        NOV = 1024,
        DECM = 2048,
        ALL_MONTHS = 4095
    };
public:
    seconds_t   _start;
    seconds_t   _end;
    seconds_t   _duration;
    seconds_t   _every;
    uint8_t     _days;
    uint16_t    _months;

    bool  isRelevantDay(uint8_t day);
    bool  isRelevantMonth(uint16_t month);
    bool  isRelevant(const TimeElements& now, const seconds_t nowSecs);

    TimerSlot() : _start(), _end(), _duration(), _every(), _days(), _months() {}

    TimerSlot( seconds_t start, 
               seconds_t end, 
               seconds_t duration, 
               seconds_t every, 
               uint8_t days,
               uint16_t months) : _start(start), _end(end), _duration(duration), _every(every), _days(days), _months(months) {}
    
    ///////////////////////////////////
	//Format: s.m.h.d.M:S
	//Example: *.5.*.1-4.*:20
	//Example details: any second on every 5th minute of every hour, from 1th to 4th week days of every month, duration 20 sec
    explicit TimerSlot(const String& schedule);
    explicit TimerSlot(const char* schedule);
    
    explicit TimerSlot(const JSONVar& json);

    JSONVar toJSON() const;
    void    print();
};

typedef std::list<TimerSlot> timerslot_list_t;
#endif