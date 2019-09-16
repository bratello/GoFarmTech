//  Convertors.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include "Convertors.h"

String read_mqtt(uint8_t* payload, unsigned int len) {
	String str;
	for(uint8_t i = 0; i < len; i++) {
		str += (char) payload[i];
	}
	return str;
}

template<>
int from_string<int>(const String& str) {
	return str.toInt();
}

template<>
long from_string<long>(const String& str) {
	return str.toInt();
}

template<>
unsigned int from_string<unsigned int>(const String& str) {
	unsigned int val = strtoul(str.c_str(), NULL, 0);
	return val;
}
template<>
unsigned long from_string<unsigned long>(const String& str) {
	unsigned long val = strtoul(str.c_str(), NULL, 0);
	return val;
}
template<>
bool from_string<bool>(const String& str) {
	return str == "1" || str.equalsIgnoreCase("true");
}
template<>
float from_string<float>(const String& str) {
	return str.toFloat();
}

template<>
String to_string<bool>(const bool& val) {
	return String(val ? "true" : "false");
}

template<>
String to_json<String>(const String& val) {
	return "\"" + val + "\"";
}

void split_string(const String& str, const String& separator, const std::function<void(const String&)> each) {
	int pos = 0;
	uint prevPos = 0;
	while((pos = str.indexOf(separator, prevPos)) != -1) {
		::yield();
		auto item = str.substring(prevPos, pos);
		item.trim();
		if(item.length() > 0)
			each(item);
		prevPos = (uint) pos + 1;
	}
	if(prevPos == 0 && pos == -1) {
		each(str);
	}
	else if(prevPos != 0 && str.length() > prevPos + 1) {
		auto item = str.substring(prevPos);
		item.trim();
		if(item.length() > 0)
			each(item);
	}
}
