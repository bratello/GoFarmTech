//  TimerTaskPipeline.h - GoFarmTech Framework Source Code
//  Author: bratello

#ifndef TIMER_TASK_PIPELINE_H
#define TIMER_TASK_PIPELINE_H
#include	<Arduino.h>
#include    "TimerTask.h"

typedef enum STEP_STATUS {
    FINISHED,
    STARTED
} step_status_t;

typedef enum STEP_COMMAND {
    CAN_FINISH_STEP,
    DO_FINISH_STEP
} step_command_t;

const   seconds_t           CONDITIONAT_STEP_DURATION = -1;
const   seconds_t           FOLLOW_BY_TASK_DURATION = -2;

class TimerTaskPipeline : protected TimerTask {
    LOGGABLE_MODULE_ID(TimerTaskPipeline)
public:
    typedef 
        std::function<step_status_t (const step_command_t&)>    
            step_status_callback_t;
    typedef 
        std::function<step_status_callback_t (const String&)>                
            step_runner_t;
    typedef
        std::function<void(const String&)>
            step_event_t;
protected:
    const   size_t              NULL_STEP = 999999999;
    const   
        static 
            step_event_t        empty_step_event;
    struct PipelineStep {
        String                  _stepName;
        seconds_t               _duration;
        step_runner_t           _stepRunner;
    };
    typedef 
        std::vector<PipelineStep>
            pipeline_step_list_t;
    pipeline_step_list_t        _steps;
    size_t                      _currentStepId = NULL_STEP;
    step_status_callback_t      _currentStepStatusCallback;
    seconds_t                   _stepStarted = 0;
    step_event_t                _onBeforeStepEvent = empty_step_event;
    step_event_t                _onAfterStepEvent = empty_step_event;
    friend class TimerValue;
protected:
    void    onStartTaskHandler();
    void    onEndTaskHandler();
    virtual	void doEndTask(const seconds_t& secondsNow = 0);
    virtual	bool canEndTask(const tmElements_t& now, const seconds_t& secondsNow);
    virtual void doStartStep(size_t stepId, const seconds_t& secondsNow);
public:
    virtual ~TimerTaskPipeline();
    TimerTaskPipeline(const String& taskName, const timer_slots_args_t& slots, const timer_predicate_t& predicate = empty_predicate);
    
    TimerTaskPipeline& operator() (const String& stepName, seconds_t duration, const step_runner_t& onRun);
    TimerTaskPipeline& operator() (const String& stepName, const step_runner_t& onRun);
    TimerTaskPipeline& onBeforeStep(const step_event_t onBefore);
    TimerTaskPipeline& onAfterStep(const step_event_t onAfter);
};

#endif