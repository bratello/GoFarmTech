//  Runnable.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"Runnable.h"

Runnable::Runnable() : _lastTime(0), _skipTime(500) {}

Runnable::~Runnable() {}

void Runnable::loop(time_t time) {
	::yield();
	if(!time) {
		time = millis();
	}
	if(time - _lastTime >= _skipTime) {
		_lastTime = time;
		doLoop();
	}
}