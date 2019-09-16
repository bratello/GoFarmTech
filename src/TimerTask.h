//  TimerTask.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef TIMER_TASK_H
#define TIMER_TASK_H
#include	<Arduino.h>
#include	<TimeLib.h>
#include	<functional>
#include	<initializer_list>
#include	"MQTTValue.h"
#include	"TimerSlot.h"


class TimerTask : public MQTTValueAbs {
	LOGGABLE_MODULE_ID(TimerTask)
public:
	typedef 	std::initializer_list<TimerSlot>		timer_slots_args_t;
	typedef 	std::function<void(void)>				timer_callback_t;
	typedef		std::function<bool(void)>				timer_predicate_t;
	const static timer_predicate_t empty_predicate;
	const static timer_callback_t  empty_callback;
public:	
	virtual String getModuleName();
protected:
	timerslot_list_t 	_slots;
	seconds_t   		_started;
	seconds_t   		_finished;
	TimerSlot*   		_currentTimerSlot;

	String				_taskName;
	bool				_enabled;
	timer_callback_t 	_onStart;
	timer_callback_t	_onEnd;
	timer_predicate_t	_predicate;

	virtual		void		doLoop() {}
	virtual 	String		getAttributeFullName(const String& attrName);
	virtual		void		doEndTask(const seconds_t& secondsNow = 0);
	virtual		void		doStartTask(const seconds_t& secondsNow = 0);
	virtual		bool		canStartTask(const tmElements_t& now, const seconds_t& secondsNow);
	virtual		bool		canEndTask(const tmElements_t& now, const seconds_t& secondsNow);
public:
	virtual   ~TimerTask();
	TimerTask(const String& taskName, const String& schedule, const timer_callback_t& onStart, const timer_callback_t& onEnd, const timer_predicate_t& predicate);
	TimerTask(const String& taskName, const timer_slots_args_t& slots, const timer_callback_t& onStart, const timer_callback_t& onEnd, const timer_predicate_t& predicate);

	virtual Description getDescription();
	virtual		void	setup();
	virtual		void	execute(const tmElements_t& now, const seconds_t& secondsNow);

	bool	isEnabled() const { return _enabled; }
	bool	enable(bool val);
};


#endif