//  SystemInfo.cpp - GoFarmTech Framework Source Code
//  Author: bratello
#include   "SystemInfo.h"
#include   "Settings.h"

#define     UPDATE_VALUE(val, exp)  { auto newVal = exp; if(newVal != val) { val = newVal; this->_client->publish(#val, val); } }

SystemInfo::SystemInfo() : MQTTValueAbs(), cpuFreqMHz(), freeHeap(), heapFragmentation(), sketchSize(), flashSize() {
    _skipTime = 10000;
}

void  SystemInfo::doLoop() {
    UPDATE_VALUE(fullVersion, ESP.getFullVersion())
    UPDATE_VALUE(coreVersion, ESP.getCoreVersion())
    UPDATE_VALUE(cpuFreqMHz, ESP.getCpuFreqMHz())
    UPDATE_VALUE(freeHeap, ESP.getFreeHeap() / 1024)
    UPDATE_VALUE(heapFragmentation, ESP.getHeapFragmentation())
    UPDATE_VALUE(sdkVersion, String(ESP.getSdkVersion()))
    UPDATE_VALUE(sketchSize, ESP.getSketchSize())
    UPDATE_VALUE(flashSize, ESP.getFlashChipSize())
    
    if(isLastErrorAvailable()) {
		_client->publish("LastError", fetchLastError());
	}
}

void  SystemInfo::setup() {
    this->_client->subscribe("skipTime", make_subscriber<time_t>([this] (const time_t& val) {
		_skipTime = val;
		this->_client->publish("skipTime", val);
	}));
}

Description SystemInfo::getDescription() {
    Description meta;
    meta.setName(getModuleName());
    meta.setType(Description::Type::number);
    meta.setAccess(Description::Access::read);
    meta.setDefaultValue(Settings::instance()->deviceVersion);
    meta.setValue(Settings::instance()->deviceVersion);

    Description skip = meta;
    skip.setName(String("skipTime"));
    skip.setAccess(Description::Access::write);
    skip.setValue(this->_skipTime);
    skip.setDefaultValue(this->_skipTime);

    Description core;
    core.setName("coreVersion");
    core.setType(Description::Type::string);
    core.setAccess(Description::Access::read);
    core.setDefaultValue(coreVersion);
    core.setValue(coreVersion);

    Description full;
    full.setName("fullVersion");
    full.setType(Description::Type::string);
    full.setAccess(Description::Access::read);
    full.setDefaultValue(fullVersion);
    full.setValue(fullVersion);

    Description cpu;
    cpu.setName("cpuFreqMHz");
    cpu.setType(Description::Type::number);
    cpu.setAccess(Description::Access::read);
    cpu.setDefaultValue(cpuFreqMHz);
    cpu.setValue(cpuFreqMHz);

    Description heap;
    heap.setName("freeHeap");
    heap.setType(Description::Type::number);
    heap.setAccess(Description::Access::read);
    heap.setDefaultValue(freeHeap);
    heap.setValue(freeHeap);

    Description heapFrag;
    heapFrag.setName("heapFragmentation");
    heapFrag.setType(Description::Type::number);
    heapFrag.setAccess(Description::Access::read);
    heapFrag.setDefaultValue(heapFragmentation);
    heapFrag.setValue(heapFragmentation);

    Description sdk;
    sdk.setName("sdkVersion");
    sdk.setType(Description::Type::string);
    sdk.setAccess(Description::Access::read);
    sdk.setDefaultValue(sdkVersion);
    sdk.setValue(sdkVersion);

    Description sketch;
    sketch.setName("sketchSize");
    sketch.setType(Description::Type::number);
    sketch.setAccess(Description::Access::read);
    sketch.setDefaultValue((long)sketchSize);
    sketch.setValue((long)sketchSize);    
    
    Description flash;
    flash.setName("flashSize");
    flash.setType(Description::Type::number);
    flash.setAccess(Description::Access::read);
    flash.setDefaultValue((long)flashSize);
    flash.setValue((long)flashSize); 

    Description lastErrorMeta;
	lastErrorMeta.setName("LastError");
	lastErrorMeta.setDefaultValue(String(""));
	lastErrorMeta.setValue(String(""));
	lastErrorMeta.setAccess(Description::Access::read);
	lastErrorMeta.setType(Description::Type::string);

    meta.addAttribute(skip);
    meta.addAttribute(lastErrorMeta);
    meta.addAttribute(core);
    meta.addAttribute(full);
    meta.addAttribute(cpu);
    meta.addAttribute(heap);
    meta.addAttribute(heapFrag);
    meta.addAttribute(sdk);
    meta.addAttribute(sketch);
    meta.addAttribute(flash);
    return meta;
}