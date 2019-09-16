//  TimerTask.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include	"TimerTask.h"

const TimerTask::timer_predicate_t TimerTask::empty_predicate = [] () { return true; };
const TimerTask::timer_callback_t  TimerTask::empty_callback = [] () {};

JSONVar make_slots_json(const timerslot_list_t& slots) {
	JSONVar json;
	int i = 0;
	for(const auto & slot : slots) {
		JSONVar slotJSON = slot.toJSON();
		slotJSON["id"] = i;
		json[i++] = slotJSON;
	}
	return json;
}

timerslot_list_t  make_slots_from_json(const String& jsonStr) {
	timerslot_list_t slots;
	auto json = JSONVar::parse(jsonStr);
	for(int i = 0; i < json.length(); i++) {
		slots.push_back(TimerSlot(json[i]));
	}
	return slots;
}

TimerTask::~TimerTask() {}

TimerTask::TimerTask(const String &taskName, 
					const String &schedule, 
					const timer_callback_t &onStart, 
					const timer_callback_t &onEnd, 
					const timer_predicate_t &predicate) : MQTTValueAbs(),
														_started(0),
														_finished(0),
														_currentTimerSlot(null),
														_taskName(taskName),
														_enabled(true),
														_onStart(onStart),
														_onEnd(onEnd),
														_predicate(predicate)
														
{
	this->_skipTime = 1000;
	auto empty = String("");
	auto persist = loadAttribute("payload", empty);
	if(persist == empty) {
		_slots.push_back(TimerSlot(schedule));
	} else {
		_slots = make_slots_from_json(persist);
	}
}

TimerTask::TimerTask(const String& taskName, 
					const timer_slots_args_t& slots, 
					const timer_callback_t& onStart, 
					const timer_callback_t& onEnd, 
					const timer_predicate_t& predicate)  : MQTTValueAbs(),
														_started(0),
														_finished(0),
														_currentTimerSlot(null),
														_taskName(taskName),
														_enabled(true),
														_onStart(onStart),
														_onEnd(onEnd),
														_predicate(predicate)
{
	this->_skipTime = 1000;
	auto empty = String("");
	auto persist = loadAttribute("payload", empty);
	if(persist == empty) {
		_slots = slots;
	} else {
		_slots = make_slots_from_json(persist);
	}
}


String TimerTask::getModuleName() {
	return _taskName;
}

Description TimerTask::getDescription() {
	Description meta;
	meta.setName(getModuleName());
	meta.setType(Description::Type::array);
	meta.setAccess(Description::Access::write);
	meta.setDefaultValue(JSONVar::parse("[]"));
	meta.setValue(make_slots_json(_slots));

	{
		Description enabledMeta;
		enabledMeta.setName("enabled");
		enabledMeta.setType(Description::Type::boolean);
		enabledMeta.setAccess(Description::Access::all);
		enabledMeta.setDefaultValue(true);
		enabledMeta.setValue(_enabled);

		meta.addAttribute(enabledMeta);
	}
	return meta;
}

String		TimerTask::getAttributeFullName(const String& attrName) {
	return getModuleName() + ":" + _taskName + ":" + attrName;
}

void	TimerTask::setup() {
	this->_enabled = loadAttribute("enabled", this->_enabled);
	this->_client->subscribe("", make_subscriber<String>([this] (const String& val) {
		_slots = make_slots_from_json(saveAttribute("payload", val));
		this->_client->publish("", val);
	}));
	this->_client->subscribe("enabled", make_subscriber<bool>([this] (const bool& val) {
		LOGGER(info("Timer ")(val ? "enabled" : "disabled"));
		enable(saveAttribute("enabled", val));
		if(!val && _started) {
			doEndTask();	
		}
	}));

	enable(_enabled);
}

bool TimerTask::enable(bool val) {
	bool prev = _enabled;
	_enabled = val;
	this->_client->publish("enabled", val);
	return prev;
}

void	TimerTask::execute(const tmElements_t& now, const seconds_t& secondsNow) {
	if(!_enabled) {
		return;
	}
	if(canEndTask(now, secondsNow)) {
		doEndTask(secondsNow);
	} else if(canStartTask(now, secondsNow)) {
		doStartTask(secondsNow);
	}
}

void	TimerTask::doEndTask(const seconds_t& secondsNow) {
	_finished = secondsNow;
	_started = 0;
	_onEnd();
	LOGGER(info("Task finished"));
	_currentTimerSlot = null;
}

void	TimerTask::doStartTask(const seconds_t& secondsNow) {
	LOGGER(info("Starting task"));
	LOGGER(_currentTimerSlot->print())
	_started = secondsNow;
	_onStart();
	LOGGER(info("Task started"));
}

bool	TimerTask::canStartTask(const tmElements_t& now, const seconds_t& secondsNow) {
	if(_started) {
		return false;
	}
	for(auto& slot : _slots) {
		if(slot.isRelevant(now, secondsNow)) {
			_currentTimerSlot = &slot;
			break;
		}
		::yield();
	}
	if(!_currentTimerSlot) {
		return false;
	}
	return (!_finished || _finished - _currentTimerSlot->_duration <= secondsNow - _currentTimerSlot->_every) && _predicate();
}

bool	TimerTask::canEndTask(const tmElements_t& now, const seconds_t& secondsNow) {
	return (_started && _currentTimerSlot->_duration + _started <= secondsNow);
}