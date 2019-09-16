//  TimerValue.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef TIMER_VALUE_H
#define TIMER_VALUE_H
#include	<map>
#include	"MQTTValue.h"
#include 	"TimerTask.h"
#include	"TimerTaskPipeline.h"


class TimerValue : public MQTTValue<time_t> {
	LOGGABLE(TimerValue)
public:
	typedef	TimerTask::timer_slots_args_t	timer_slots_args_t;
	typedef TimerTask::timer_callback_t		timer_callback_t;
	typedef TimerTask::timer_predicate_t	timer_predicate_t;
protected:
	typedef	MQTTValue<time_t>				MQTTTimeValue_t;
	typedef std::map<String, TimerTask*>	timer_task_map_t;

	timer_task_map_t						_tasks;
	event_handle_t							_onChangeHandle;
public:
	TimerValue();

	virtual		void	setup();
	virtual		void	doLoop();
	virtual Description getDescription();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Format: s.m.h.d.M:S
	// Example: *.5.*.5-10.*:20
	// Example details: any second on every 5th minute of every hour, from 5th to 10th days of every month, duration 20 sec
	void				on(const String& taskName, const String& schedule, const timer_callback_t& onStart, const timer_callback_t& onEnd, const timer_predicate_t& predicate = TimerTask::empty_predicate);
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// slots initializer list format - { {0, TIME_INTERVAL::DAY, 20, 60, TimerSlot::ALL_DAYS, TimerSlot::ALL_MONTHS} } 
	void				on(const String& taskName, const timer_slots_args_t& slots, const timer_callback_t& onStart, const timer_callback_t& onEnd, const timer_predicate_t& predicate = TimerTask::empty_predicate);
	timer_callback_t	onImmediateOnce(const String& taskName, int duration, const timer_callback_t& onStart, const timer_callback_t& onEnd, const timer_predicate_t& predicate = TimerTask::empty_predicate);
	TimerTaskPipeline&  onPipeline(const String& taskName, const timer_slots_args_t& slots, const timer_predicate_t& predicate = TimerTask::empty_predicate);
	void				off(const String& taskName);
	bool				isEnabledTask(const String& taskName);
	bool				enableTask(const String& taskName, bool val);
};

typedef TimerValue& Timer;

#endif