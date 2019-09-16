//  Loggable.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef LOGGABLE_H
#define LOGGABLE_H

#include	<Arduino.h>
#include	<IPAddress.h>
#include	<set>
#include	<functional>
#include	<TimeLib.h>
#include	"Modules.h"

class Logger {
protected:
	String			_data;
	String			_moduleName;
	uint			_moduleID;
	uint			_error;
	time_t			_now;
	bool			_warn;
public:
	~Logger();

	template<class T>
	Logger(const T& data, uint error = 0, const String& moduleName = "", uint moduleID = 0, bool warning = false)  : _data(data),
																								_moduleName(moduleName),
																								_moduleID(moduleID),
																								_error(error),
																								_now(now()),
																								_warn(warning) { }

	Logger() : _moduleID(), _error(), _now(), _warn() {}

	template<class T>
    Logger& operator() (const T& p) {
		auto v = String(p);
		if(!v.length()) {
			return *this;
		}
    	_data += v;
		return *this;
    }

    template<class T>
    Logger& operator+ (const T& p) {
    	return (*this)(p);
    }

    operator String ();
    bool	isError();
	bool	isWarning();
};

template<>
Logger& Logger::operator()<IPAddress>(const IPAddress& p);

class Loggable {
public:
	typedef	std::set<int>		module_set_t;
	typedef std::function<void(const String&)>	output_t;	
protected:
	virtual String 	getModuleName();
	virtual uint 	getModuleID();

	Loggable();
	virtual	~Loggable();

	template<class T>
	Logger	info(const T& v) {
		return Logger(v, 0, getModuleName(), getModuleID());
	}

	template<class T>
	Logger	error(uint error, const T& v) {
		return Logger(v, error, getModuleName(), getModuleID());
	}

	template<class T>
	Logger	warn(uint error, const T& v) {
		return Logger(v, error, getModuleName(), getModuleID(), true);
	}

	void	enableModulesLog(const module_set_t& modules);
	void	muteModulesLog(const module_set_t& modules);
	void	enableModuleLog(const int& id);
	void	muteModuleLog(const int& id);
	void	addLogOutput(const output_t& out);

	bool	isLastErrorAvailable();
	String	fetchLastError(bool reset = true);

	void	logMemUsage(const char* msg = nullptr);
};

#define LOGGABLE_MODULE_NAME(className)	public: virtual String getModuleName() { return #className; }
#define LOGGABLE_MODULE_ID(className)  	public: virtual uint getModuleID() { return static_cast<uint>(Modules::className); }
#define LOGGABLE_ID(id)  			public: virtual uint getModuleID() { return static_cast<uint>(id); }
#define LOGGABLE(className) 		LOGGABLE_MODULE_NAME(className) LOGGABLE_MODULE_ID(className)

//#ifdef  DEBUG
#define LOGGER(exp)					exp;
//#else
//#define LOGGER(exp)
//#endif

#endif
