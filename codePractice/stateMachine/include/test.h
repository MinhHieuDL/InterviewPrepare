#pragma once

#include <vector>
#include "fwStateMachine.h"

typedef enum{
    STATE_INIT = 0,
    STATE_RUNNING,
    STATE_STOPPED,
    STATE_ERROR
} StateID;    

typedef enum {
    NO_ACTION = 0,
    ACTION_ENTER_INIT,
    ACTION_EXIT_INIT,
    ACTION_ENTER_RUNNING,
    ACTION_EXIT_RUNNING,
    ACTION_ENTER_STOPPED,
    ACTION_EXIT_STOPPED,
    ACTION_ENTER_ERROR,
    ACTION_EXIT_ERROR,

    ACTION_WARNING_ERROR,
    ACTION_RESET
} ActionID;

typedef enum {
    EVENT_START = 0,
    EVENT_STOP,
    EVENT_ERROR,
    EVENT_RESET
} EventID;

// Define event control list for each state
const std::vector<STATE_EVENT_CONTROL> EventControlInitList = {
    {EVENT_START, STATE_RUNNING, NO_ACTION},
    {EVENT_ERROR, STATE_ERROR, ACTION_WARNING_ERROR},    
};  

const std::vector<STATE_EVENT_CONTROL> EventControlRunningList = {
    {EVENT_STOP, STATE_STOPPED, NO_ACTION},
    {EVENT_ERROR, STATE_ERROR, ACTION_WARNING_ERROR},
};

const std::vector<STATE_EVENT_CONTROL> EventControlStoppedList = {
    {EVENT_START, STATE_RUNNING, NO_ACTION},
    {EVENT_ERROR, STATE_ERROR, ACTION_WARNING_ERROR},
    {EVENT_RESET, STATE_INIT, NO_ACTION}
};

const std::vector<STATE_EVENT_CONTROL> EventControlErrorList = {
    {EVENT_RESET, STATE_INIT, NO_ACTION}
};

// Define the states with their respective event control lists 
const std::vector<STATE> TestStateList = {
    {STATE_INIT, ACTION_ENTER_INIT, ACTION_EXIT_INIT, EventControlInitList},
    {STATE_RUNNING, ACTION_ENTER_RUNNING, ACTION_EXIT_RUNNING, EventControlRunningList},
    {STATE_STOPPED, ACTION_ENTER_STOPPED, ACTION_EXIT_STOPPED, EventControlStoppedList},
    {STATE_ERROR, ACTION_ENTER_ERROR, ACTION_EXIT_ERROR, EventControlErrorList}
};

class Test : public CStateMachine {
    public:
        Test() = default;
        virtual ~Test() = default;
    
    protected:
        void CallMapAction(unsigned char ucActionID) override;
        
    private:
        void EnterInitAction();
        void ExitInitAction();
        void EnterRunningAction();
        void ExitRunningAction();
        void EnterStoppedAction();
        void ExitStoppedAction();
        void EnterErrorAction();
        void ExitErrorAction();

        void WarningErrorAction();
        void ResetAction();
};