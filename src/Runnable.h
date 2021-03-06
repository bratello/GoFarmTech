//  Runnable.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef RUNNABLE_H
#define RUNNABLE_H
#include	<Arduino.h>
#include	<TimeLib.h>
#include	<memory>

class Runnable {
protected:
	time_t	_lastTime;
	time_t	_skipTime;
protected:
	virtual		void	doLoop() = 0;
public:
	Runnable();
	virtual		~Runnable();
	virtual		void	setup() = 0;
	virtual		void	loop(time_t time = 0);
	
public:
	inline		time_t	nextLoop() {
		return _lastTime + _skipTime;
	}
};
typedef std::unique_ptr<Runnable>	RunnablePtr;

#endif