//  TimerTaskPipeline.cpp - GoFarmTech Framework Source Code
//  Author: bratello

#include    "TimerTaskPipeline.h"

const TimerTaskPipeline::step_event_t TimerTaskPipeline::empty_step_event = [](const String&){};

TimerTaskPipeline::~TimerTaskPipeline() {}

TimerTaskPipeline::TimerTaskPipeline(const String& taskName, const timer_slots_args_t& slots, const timer_predicate_t& predicate) : TimerTask(
    taskName, 
    slots, 
    std::bind(&TimerTaskPipeline::onStartTaskHandler, this), 
    std::bind(&TimerTaskPipeline::onEndTaskHandler, this), 
    predicate), _currentStepId(0), _stepStarted(0) {

}

void TimerTaskPipeline::onStartTaskHandler() {
    doStartStep(0, _started);
}

void TimerTaskPipeline::onEndTaskHandler() {
    _stepStarted = 0;
    _currentStepId = NULL_STEP;
    _currentStepStatusCallback = step_status_callback_t();
}

void TimerTaskPipeline::doEndTask(const seconds_t& secondsNow) {
    if(_currentStepId != NULL_STEP && _currentStepStatusCallback) {
        _currentStepStatusCallback(step_command_t::DO_FINISH_STEP);
    }
    TimerTask::doEndTask(secondsNow);
}

bool TimerTaskPipeline::canEndTask(const tmElements_t& now, const seconds_t& secondsNow) {
    if(!_started) {
        return false;
    }
    if(_currentStepId == NULL_STEP) {
        return true;
    }
    if(_currentStepId < _steps.size()) {
        auto& step = _steps[_currentStepId];
        if(step._duration == CONDITIONAT_STEP_DURATION) {
            if(step_status_t::FINISHED == _currentStepStatusCallback(step_command_t::CAN_FINISH_STEP)) {
                doStartStep(_currentStepId + 1, secondsNow);
            }
        } else {
            auto duration = (step._duration == FOLLOW_BY_TASK_DURATION ? _currentTimerSlot->_duration : step._duration);
            if(_stepStarted + duration >= secondsNow) {
                _currentStepStatusCallback(step_command_t::DO_FINISH_STEP);
                doStartStep(_currentStepId + 1, secondsNow);
            }
        }
        return (_currentStepId == NULL_STEP);
    }
    return true;
}

void TimerTaskPipeline::doStartStep(size_t stepId, const seconds_t& secondsNow) {
    if(_currentStepId >= 0 && _currentStepId < _steps.size()) {
        auto prevStepName = _taskName + "." + _steps[_currentStepId]._stepName;
        _onAfterStepEvent(prevStepName);
    }

    if(stepId == NULL_STEP || stepId >= _steps.size()) {
        _currentStepId = NULL_STEP;
        _stepStarted = 0;
        _currentStepStatusCallback = step_status_callback_t();
        LOGGER(info("Task ")(_taskName)(", last step reached"))
        return;
    }
    _currentStepId = stepId;
    _stepStarted = secondsNow;
    auto& step = _steps[_currentStepId];
    auto fullStepName = _taskName + "." + step._stepName;
    LOGGER(info("Step ")(fullStepName)(" starting"))
    _onBeforeStepEvent(fullStepName);
    _currentStepStatusCallback = step._stepRunner(fullStepName);
}

TimerTaskPipeline& TimerTaskPipeline::operator() (const String& stepName, seconds_t duration, const step_runner_t& onRun) {
    _steps.push_back({stepName, duration, onRun});
    return *this;
}

TimerTaskPipeline& TimerTaskPipeline::operator() (const String& stepName, const step_runner_t& onRun) {
    _steps.push_back({stepName, CONDITIONAT_STEP_DURATION, onRun});
    return *this;
}

TimerTaskPipeline& TimerTaskPipeline::onBeforeStep(const step_event_t onBefore) {
    _onBeforeStepEvent = onBefore;
    return *this;
}
    
TimerTaskPipeline& TimerTaskPipeline::onAfterStep(const step_event_t onAfter) {
    _onAfterStepEvent = onAfter;
    return *this;
}