//  Loggable.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include <list>
#include "Loggable.h"
#include "Convertors.h"


typedef std::list<Loggable::output_t>	outputs_t;

static	Loggable::module_set_t	_mutedModules;
static	Loggable::module_set_t	_enabledModules;
static	Logger					_lastError;
static	outputs_t				_outputs;

void print_logs(const String& str) {
	for(const auto& output : _outputs) {
		output(str);
	}
}

Logger::~Logger() {
	if(!_data.length()) {
		return;
	}
	
	if(!isError()) {
		if(!_mutedModules.empty()) {
			//Mute log for _moduleID
			if(_mutedModules.find(_moduleID) != _mutedModules.end()) {
				return;
			}
		}

		if(!_enabledModules.empty()) {
			//Mute log for all modules except Loggable::_enabledModules
			if(_enabledModules.find(_moduleID) == _enabledModules.end()) {
				return;
			}
		}
	} else {
		_lastError = *this;
	}
	print_logs((String)*this);
}

Logger::operator String () {
	String format = ((_error) ? String(_warn ? "[WARN(" : "[ERROR(") + _error + ")]\t" : String("[INFO]\t\t"));
	format += print_time(_now) + "\t";
	String  name = (_moduleID > 0 && _moduleName.length() > 0)? _moduleName + "(#" + _moduleID + ") " : String("");
	for(int i = name.length() / 5; i < 5; i++) {
		name += "\t";
	}
	format += name + _data;
	return format;
}

bool Logger::isError() { 
	return _error > 0;
}

bool Logger::isWarning() {
	return _warn;
}

template<>
Logger& Logger::operator()<IPAddress>(const IPAddress& p) {
	return (*this)(p.toString());
}

Loggable::Loggable() {}

Loggable::~Loggable() {}

String Loggable::getModuleName() {
	return "";
}

uint Loggable::getModuleID() {
	return 0;
}

void	Loggable::enableModulesLog(const module_set_t& modules) {
	_enabledModules = modules;
	_mutedModules.clear();
}

void	Loggable::muteModulesLog(const module_set_t& modules) {
	_enabledModules.clear();
	_mutedModules = modules;
}

void do_enable(Loggable::module_set_t& enable, Loggable::module_set_t& mute, const int& id) {
	if(mute.find(id) != mute.end()) {
		mute.erase(id);
	}
	if(enable.find(id) == enable.end()) {
		enable.insert(id);
	}
}

void	Loggable::enableModuleLog(const int& id) {
	do_enable(_enabledModules, _mutedModules, id);
}

void	Loggable::muteModuleLog(const int& id) {
	do_enable(_mutedModules, _enabledModules, id);
}

bool	Loggable::isLastErrorAvailable() {
	return _lastError.isError();
}

String  Loggable::fetchLastError(bool reset) {
	String msg = _lastError;
	if(reset) {
		_lastError = Logger();
	}
	msg.replace("\t", " ");
	return msg;
}

void	Loggable::addLogOutput(const output_t& out) {
	_outputs.push_back(out);
}

void	Loggable::logMemUsage(const char* msg) {
	static float freeMem = 0;
	float memUsage = (ESP.getFreeHeap() / 1024);
	yield();
	if(freeMem == memUsage) {
		return;
	}
	freeMem = memUsage;
	info("Free heap size")(msg != nullptr ? " ": "")(msg)(": ")(String(freeMem, 1))(" KB");
}