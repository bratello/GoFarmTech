//  TimerSlot.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"TimerSlot.h"
#include    "Convertors.h"

const static std::initializer_list<const char*> WEEK_DAYS = {"su", "mo", "tu", "we", "th", "fr", "sa"};
const static std::initializer_list<const char*> MONTHS = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};

inline String printDasyMonths(std::vector<const char*> names, const uint16_t& val) {
    ::yield();
    int len = names.size();
    for(int i = 0; i < len; i++) {
        if(!(val & (1 << i))) {
            names.erase(names.begin() + (i - (len - names.size())));
        }
    }
    if(names.empty()) {
        return "none";
    }
    return std::accumulate(names.begin(), names.end(), String(), [] (const String& init, const String& item) {
        return init + (init.length() > 0 ? "," : "") + item;
    });
}

template<typename R>
inline R jsonNamesToNumber(JSONVar& var, const std::vector<const char*>& names) {
    ::yield();
    R ret = R();
    int len = names.size();
    for(int i = 0; i < len; i++) {
        ret += ((bool)var[names[i]])? 1 << i : 0;
    }
    return ret;
}

template<typename T>
inline JSONVar numberToJsonNames(const T& val, const std::vector<const char*>& names) {
    ::yield();
    JSONVar ret = JSONVar();
    int len = names.size();
    for(int i = 0; i < len; i++) {
        ret[names[i]] = (val & 1 << i) ? true : false;
    }
    return ret;
}

inline String printDays(const uint8_t& val) {
    return printDasyMonths(WEEK_DAYS, val);
}

inline String printMonths(const uint16_t& val) {
    return printDasyMonths(MONTHS, val);
}

inline uint8_t jsonDaysToNumber(JSONVar& var) {
    return jsonNamesToNumber<uint8_t>(var, WEEK_DAYS);
}

inline uint16_t jsonMonthsToNumber(JSONVar& var) {
    return jsonNamesToNumber<uint16_t>(var, MONTHS);
}

inline JSONVar numberToJsonDays(const uint8_t& val) {
    return numberToJsonNames(val, WEEK_DAYS);
}

inline JSONVar numberToJsonMonths(const uint16_t& val) {
    return numberToJsonNames(val, MONTHS);
}

bool  TimerSlot::isRelevantDay(uint8_t day) {
    return (1 << (day - 1)) & _days;
}

bool  TimerSlot::isRelevantMonth(uint16_t month) {
    return (1 << (month - 1)) & _months;
}

bool  TimerSlot::isRelevant(const TimeElements& now, const seconds_t nowSecs) {
    if(nowSecs >= _start && nowSecs <= _end) {
        return isRelevantDay(now.Wday) && isRelevantMonth(now.Month);
    }
    return false;
}

TimerSlot::TimerSlot(const char* schedule) : TimerSlot(String(schedule)) {}

TimerSlot::TimerSlot(const String& schedule) :_start(), _end(), _duration(), _every(), _days(), _months() {
    int i = 0;
    _start = 0;
    _end = 0;
    struct {
        bool everySec = false;
        bool rangeSec = false;
        bool everyHour = false;
        bool rangeHour = false;
        bool everyMin = false;
        bool rangeMin = false;

        long fromSec = 0;
        long toSec = 0;
        long fromHour = 0;
        long toHour = 0;
        long fromMin = 0;
        long toMin = 0;
    } limits;
    split_string(schedule, ".", [this, &i, &limits] (const String& item) {
		String token = item;
        token.trim();
		LOGGER(info("Token: ") + token)
		auto secStart = token.indexOf(":");
		if(secStart != -1) {
			_duration = token.substring(secStart + 1).toInt();
			LOGGER(info("Duration: ") + _duration)
			token = token.substring(0, secStart);
			token.trim();
		}

		auto minus = token.indexOf("-");
        bool isRange = minus != -1;
		auto to = (isRange ? token.substring(minus + 1).toInt() : 0);
        bool isAny = (token == "*");
		auto from =  (isRange ? token.substring(0, minus).toInt() : (isAny? 0 : token.toInt()));
		LOGGER(info("Range to: ")(to)(", from: ")(from))
        if(i == 0) {
            LOGGER(info("Placeholder - seconds"))
            if(isAny) {
                limits.everySec = true;
            } else if(isRange) {
                limits.rangeSec = true;
                limits.fromSec = from;
                limits.toSec = to;
            } else {
                limits.fromSec = from;
            }
        } else if(i == 1) {
            LOGGER(info("Placeholder - minutes"))
            if(isAny) {
                limits.everyMin = true;
            } else if(isRange) {
                limits.rangeMin = true;
                limits.fromMin = from;
                limits.toMin = to;
            } else {
                limits.fromMin = from;
            }
            
        } else if(i == 2) {
            LOGGER(info("Placeholder - hours"))
            if(isAny) {
                limits.everyHour = true;
            } else if(isRange) {
                limits.rangeHour = true;
                limits.fromHour = from;
                limits.toHour = to;
            } else {
                limits.fromHour = from;
            }            
        } else if(i == 3) {
            LOGGER(info("Placeholder - days"))
            if(isAny) {
                _days = WEEK_DAYS_VALUE::ALL_DAYS;
            } else if(isRange){
                auto min = from > to ? to : from;
                auto max = from < to ? to : from;
                for(int k = min; k < max + 1; k++) {
                    _days += 1 << k;
                }
            } else {
                _days = 1 << from;
            }
        } else if(i == 4) {
            LOGGER(info("Placeholder - months"))
            if(isAny) {
                _months = MONTHS_VALUE::ALL_MONTHS;
            } else if(isRange){
                auto min = from > to ? to : from;
                auto max = from < to ? to : from;
                for(int k = min; k < max + 1; k++) {
                    _months += 1 << k;
                }
            } else {
                _months = 1 << from;
            }
        }
        i++;
	});
    
    if(!limits.everySec) {
        if(!limits.rangeSec) {
            _every = limits.fromSec;
        } else {
            _start = limits.fromSec;
            _end = limits.toSec;
        }
    }
    
    if(!limits.everyMin) {
        if(!limits.rangeMin) {
            _every += limits.fromMin * TIME_INTERVAL::MINUTE;
        } else {
            _start += limits.fromMin * TIME_INTERVAL::MINUTE;
            _end += limits.toMin * TIME_INTERVAL::MINUTE;
        }
    } else {
        if(!_every)
            _every = TIME_INTERVAL::MINUTE;
    }

    if(!limits.everyHour) {
        if(!limits.rangeHour) {
            _every += limits.fromHour * TIME_INTERVAL::HOUR;
        } else {
            _start += limits.fromHour * TIME_INTERVAL::HOUR;
            _end += limits.toHour * TIME_INTERVAL::HOUR;
        }
    } else {
        if(!_every)
            _every = TIME_INTERVAL::HOUR;
    }

    if(!_end) {
        _end = TIME_INTERVAL::DAY;
    }
    if(!_every) {
        _every = (_duration > TIME_INTERVAL::MINUTE ? _duration + TIME_INTERVAL::MINUTE : TIME_INTERVAL::MINUTE);
    }
    print();
}

void    TimerSlot::print() {
    info("Timeslot scheduled: ")
        ("start=")(_start)
        (", end=")(_end)
        (", duration=")(_duration)
        (", every=")(_every)
        (", days=")(printDays(_days)) 
        (", months=")(printMonths(_months));
}

TimerSlot::TimerSlot(const JSONVar& json) {
    _start = json["st"];
    _end = json["en"];
    _every = json["ev"];
    _duration = json["dur"];
    {
        _days = json["ds"];
    }
    {
        _months = json["mns"];
    }
}

JSONVar TimerSlot::toJSON() const {
    JSONVar obj;

    obj["st"] = _start;
    obj["en"] = _end;
    obj["ev"] = _every;
    obj["dur"] = _duration;

    //JSONVar days = numberToJsonDays(_days);
    obj["ds"] = _days;

    //JSONVar months = numberToJsonMonths(_months);
    obj["mns"] = _months;
    return obj;
}