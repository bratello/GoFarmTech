//  Observable.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef Observable_h
#define Observable_h

#include <functional>
#include <map>

template<class T>
class Observable {
public:
    typedef             Observable<T>                                   observable_t;
    typedef             size_t                                          event_handle_t;
    typedef             std::function<void(T,T)>                        event_t;
    typedef             std::function<void()>                           simple_event_t;
    typedef             std::function<bool(T,T)>                        conditional_event_t;
    typedef             std::pair<conditional_event_t, event_t>         event_holder_t;
    typedef             std::map<event_handle_t, event_holder_t>        events_t;
    typedef             std::map<event_handle_t, simple_event_t>        simple_events_t;
protected:
    events_t            _onChangedEvents;
    simple_events_t     _onMaxValReachedEvents;
    simple_events_t     _onMinValReachedEvents;
    simple_events_t     _onOKValReachedEvents;
    T                   _val = T(), _minVal = T(), _maxVal = T(), _minMaxOff = T();
    bool                _changed = false;
    bool                _enabled = true;

    void cleaup() {
        _onChangedEvents.clear();
        _onMaxValReachedEvents.clear();
        _onMinValReachedEvents.clear();
        _onOKValReachedEvents.clear();
        _changed = false;
    }

    void raiseEvents(const events_t& handlers, T newVal, T oldVal) {
        for(auto& it : handlers) {
            auto entry = it.second;
            if(entry.first != NULL) {
                if(entry.first(newVal, oldVal)) {
                    entry.second(newVal, oldVal);
                }
            } else {
                entry.second(newVal, oldVal);
            }
        }
    }

    void raiseSimpleEvents(const simple_events_t& handlers) {
        for(auto& it : handlers) {
            it.second();
        }
    }

    void doChange(T newVal, T oldVal) {
        if(newVal == oldVal) {
            return;
        }
        if(!isEnabled()) {
            return;
        }
        _changed = true;
        _val = newVal;
        raiseEvents(_onChangedEvents, newVal, oldVal);
        if(_maxVal != _minMaxOff && oldVal < _maxVal && newVal >= _maxVal) {
            raiseSimpleEvents(_onMaxValReachedEvents);
        } else if(_minVal != _minMaxOff && oldVal > _minVal && newVal <= _minVal) {
            raiseSimpleEvents(_onMinValReachedEvents);
        } else if(
            _maxVal != _minMaxOff
            && _minVal != _minMaxOff
            && (oldVal <= _minVal || oldVal >= _maxVal) 
            && (newVal > _minVal && newVal < _maxVal)
        ) {
            raiseSimpleEvents(_onOKValReachedEvents);
        }
    }

    template<class C>
    event_handle_t getNewEventHandlerId(const C& handlers) {
        event_handle_t newId = { std::rand() + handlers.size() };
        while(handlers.find(newId) != handlers.end()) {
            newId = getNewEventHandlerId(handlers);
        }
        return newId;
    }
public:
    Observable(T v = T(), T min = T(), T max = T()) {
        _val = v;
        _minVal = min;
        _maxVal = max;
        _changed = false;
        _enabled = true;
    }
    
    observable_t& operator=(const T& v) {
        doChange(v, _val);
        return *this;
    }

    observable_t& setValue(const T& v) {
        return this->operator=(v);
    }

    observable_t& setMinValue(const T& v) {
        _minVal = v;
        return *this;
    }

    observable_t& setMaxValue(const T& v) {
        _maxVal = v;
        return *this;
    }
    
    operator T () const {
        return _val;
    }

    T getValue() const {
        return _val;
    }

    T getMinValue() const {
        return _minVal;
    }

    T getMaxValue() const {
        return _maxVal;
    }
    
    operator event_t () {
        return [this](T newVal, T oldVal) {
            *this = newVal;
        };
    }

    bool isMaxValue() {
        return _maxVal != _minMaxOff && _val >= _maxVal;
    }

    bool isMinValue() {
        return _minVal != _minMaxOff && _val <= _maxVal;
    }

    bool isOKValue() {
        return !isMaxValue() && !isMinValue();
    }

    virtual bool	isEnabled() const { 
        return _enabled; 
    }

    virtual bool	enable(bool val) {
        bool temp = _enabled;
        _enabled = val;
        return temp;
    }
    
    event_handle_t onChanged (const event_t& handler, const conditional_event_t& conditional = NULL) {
        auto newId = getNewEventHandlerId(_onChangedEvents);
        _onChangedEvents[newId] = std::make_pair(conditional, handler);
        return newId;
    }
    
    observable_t& offChanged (const event_handle_t& handle) {
        auto it = _onChangedEvents.find(handle);
        if(it != _onChangedEvents.end()) {
            _onChangedEvents.erase(it);
        }
        return *this;
    }

    event_handle_t onMaxValReached (const simple_event_t& handler) {
        auto newId = getNewEventHandlerId(_onMaxValReachedEvents);
        _onMaxValReachedEvents[newId] = handler;
        return newId;
    }
    
    observable_t& offMaxValReached (const event_handle_t& handle) {
        auto it = _onMaxValReachedEvents.find(handle);
        if(it != _onMaxValReachedEvents.end()) {
            _onMaxValReachedEvents.erase(it);
        }
        return *this;
    }

    event_handle_t onMinValReached (const simple_event_t& handler) {
        auto newId = getNewEventHandlerId(_onMinValReachedEvents);
        _onMinValReachedEvents[newId] = handler;
        return newId;
    }
    
    observable_t& offMinValReached (const event_handle_t& handle) {
        auto it = _onMinValReachedEvents.find(handle);
        if(it != _onMinValReachedEvents.end()) {
            _onMinValReachedEvents.erase(it);
        }
        return *this;
    }

    event_handle_t onOKValReached (const simple_event_t& handler) {
        auto newId = getNewEventHandlerId(_onOKValReachedEvents);
        _onOKValReachedEvents[newId] = handler;
        return newId;
    }
    
    observable_t& offOKValReached (const event_handle_t& handle) {
        auto it = _onOKValReachedEvents.find(handle);
        if(it != _onOKValReachedEvents.end()) {
            _onOKValReachedEvents.erase(it);
        }
        return *this;
    }
};

#endif /* Observable_h */
