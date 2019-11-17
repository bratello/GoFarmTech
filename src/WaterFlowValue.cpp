//  WaterFlowValue.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include "WaterFlowValue.h"

WaterFlowValue::WaterFlowValue(uint8_t pin, ulong val, ulong min, ulong max) : WaterFlowValueBase(val, min, max), _pin(pin) {
    _skipTime = 1000;
}

void WaterFlowValue::doLoop() {
    auto val = getWaterUsage();
    if(val) {
        setValue(val);
    }
    WaterFlowValueBase::doLoop();
}

void	WaterFlowValue::loop(time_t time) {
    auto temp = _lastTime;
    WaterFlowValueBase::loop(time);
    if(temp != _lastTime) {
        _prevTime = temp;
    }
    return;
}

void	WaterFlowValue::setup() {
    WaterFlowValueBase::setup();
    setupValueReachedEvents();
}

void    WaterFlowValue::setupPinInterrupt(void (*f)(void)) {
    pinMode(_pin, INPUT_PULLUP);
    //digitalWrite(_pin, HIGH); // Optional Internal Pull-Up
    attachInterrupt(digitalPinToInterrupt(_pin), f, RISING);
    //sei();
}

ulong   WaterFlowValue::calcWaterUsage(ulong counter, float flowFactor, bool inMilliLiters) {
    auto flowRate = (( 1000.0 / ( _lastTime - _prevTime)) * counter) / flowFactor;
    return (flowRate / 60 ) * (inMilliLiters ? 1000 : 1);
}

void    WaterFlowValue::setupValueReachedEvents() {
    this->onChanged( [this] (ulong newVal, ulong oldVal) {
        yield();
        value_reached_events_t::iterator it = _value_reached_events.begin();
        while (it != _value_reached_events.end()) {
            if(it->_started + it->_required >= newVal) {
                it->_event(newVal);
                it = _value_reached_events.erase(it);
            } else {
                it++;
            }
        }
        return;
    });
}

void    WaterFlowValue::onValueFromNowReached(ulong required, const value_reached_event_t& callback) {
    _value_reached_events.push_back({ getValue(), required, callback });
}