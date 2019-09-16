//  TimerValue.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include 	"TimerValue.h"

TimerValue::TimerValue() : MQTTTimeValue_t(), _onChangeHandle(0) {
	this->_skipTime = 1000;
}

void	TimerValue::setup() {
	Observable::cleaup();
	for(auto it : _tasks) {
		it.second->setClient(_client);
		it.second->setup();
	}
	if(_onChangeHandle) {
		offChanged(_onChangeHandle);
	}
	_onChangeHandle = onChanged([this] (time_t newVal, time_t oldVal) {
		LOGGER(info("Time changed from server: ")(print_time(newVal)))
		setTime(newVal);
	});
	MQTTTimeValue_t::setup();
}

void	TimerValue::doLoop() {
	if(!isEnabled()) {
		return;
	}
	auto nowTime = make_time_elements(_lastTime / 1000);
	auto secondsNow = make_seconds(nowTime);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Copy tasks before iteration, some of the callbacks can change the _tasks map in some of the task callbacks
	timer_task_map_t tasksCopy = _tasks;
	for(auto it : tasksCopy) {
		it.second->execute(nowTime, secondsNow);
	}
	MQTTTimeValue_t::doLoop();
}

Description TimerValue::getDescription() {
	Description meta;
	meta.setName(getModuleName());
	meta.setType(Description::Type::number);
	meta.setAccess(Description::Access::write);
	meta.setDefaultValue(0);
	meta.setValue(now());
	
	Description enabledMeta;
	enabledMeta.setName("enabled");
	enabledMeta.setType(Description::Type::boolean);
	enabledMeta.setAccess(Description::Access::all);
	enabledMeta.setDefaultValue(true);
	enabledMeta.setValue(isEnabled());
	meta.addAttribute(enabledMeta);
	for(timer_task_map_t::iterator it = _tasks.begin(); it != _tasks.end(); it++) {
		meta.addChild(it->second->getDescription());
	}
	return meta;
}

void	TimerValue::on(const String& taskName, const String& schedule, const timer_callback_t& onStart, const timer_callback_t& onEnd, const timer_predicate_t& predicate) {
	auto task = new TimerTask(taskName, schedule, onStart, onEnd, predicate);
	_tasks[taskName] = task;
}

void	TimerValue::on(const String& taskName, const timer_slots_args_t& slots, const timer_callback_t& onStart, const timer_callback_t& onEnd, const timer_predicate_t& predicate) {
	auto task = new TimerTask(taskName, slots, onStart, onEnd, predicate);
	_tasks[taskName] = task;
}


TimerValue::timer_callback_t	TimerValue::onImmediateOnce(const String& taskName, int duration, const timer_callback_t& onStart, const timer_callback_t& onEnd, const timer_predicate_t& predicate) {
	return [this, taskName, duration, onStart, onEnd, predicate] () {
		if(predicate && !predicate()) {
			//Do not run this task
			return;
		}
		String schedule = String("*.*.*.*.*:") + duration;
		auto task = new TimerTask(taskName, schedule, onStart, [this, taskName, onEnd] () {
			onEnd();
			off(taskName);
		}, TimerTask::empty_predicate);
		_tasks[taskName] = task;
	};
}

bool		TimerValue::isEnabledTask(const String& taskName) {
	auto it = _tasks.find(taskName);
	if(_tasks.end() != it) {
		return it->second->isEnabled();
	}
	return false;
}

bool		TimerValue::enableTask(const String& taskName, bool val) {
	auto it = _tasks.find(taskName);
	if(_tasks.end() != it) {
		return it->second->enable(val);
	}
	return false;
}

void	TimerValue::off(const String& taskName) {
	auto it = _tasks.find(taskName);
	if(_tasks.end() != it) {
		delete it->second;
		_tasks.erase(it);
	}
}

TimerTaskPipeline&  TimerValue::onPipeline(const String& taskName, const timer_slots_args_t& slots, const timer_predicate_t& predicate) {
	auto task = new TimerTaskPipeline(taskName, slots, predicate);
	_tasks[taskName] = task;
	return *task;
}