//  Convertors.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef CONVERTORS_H
#define CONVERTORS_H
#include	<Arduino.h>
#include	<TimeLib.h>
#include	<functional>
#include 	<numeric>

#define DOIT_ONCE(exp, period)	{ static int ___times = 0; if(period < ___times++) {___times = 0; exp; } }

String read_mqtt(uint8_t* payload, unsigned int len);

template<typename T>
T from_string(const String& str) {
	return str;
}

template<>
int from_string<int>(const String& str);
template<>
long from_string<long>(const String& str);
template<>
unsigned int from_string<unsigned int>(const String& str);
template<>
unsigned long from_string<unsigned long>(const String& str);
template<>
bool from_string<bool>(const String& str);
template<>
float from_string<float>(const String& str);


template<typename T>
T from_mqtt(uint8_t* data, unsigned int len) {
	return from_string<T>(read_mqtt(data, len));
}

template<typename T>
String to_string(const T& val) {
	return String(val);
}

template<>
String to_string<bool>(const bool& val);


template<typename T>
String to_json(const T& val) {
	return to_string<T>(val);
}

template<>
String to_json<String>(const String& val);

void split_string(const String& str, const String& separator, const std::function<void(const String&)> each);

template<typename Tp, typename C, typename BinaryOp>
Tp accumulate_chain(const C& container, BinaryOp op) {
	return std::accumulate(container.begin(), container.end(), Tp(), op);
}

inline TimeElements make_time_elements(const time_t& t) {
    TimeElements elementsNow;
    ::breakTime(t, elementsNow);
    return elementsNow;
}

#define PRINT_TIME_DIGIT(digit) (digit < 10 ? String("0") + digit : String(digit))

inline String print_time(const time_t& t) {
	TimeElements tm = make_time_elements(t);
	return String(tmYearToCalendar(tm.Year)) + "-" + PRINT_TIME_DIGIT(tm.Month) + "-" + PRINT_TIME_DIGIT(tm.Day) + " " + PRINT_TIME_DIGIT(tm.Hour) + ":" + PRINT_TIME_DIGIT(tm.Minute) + ":" + PRINT_TIME_DIGIT(tm.Second);
}

#endif