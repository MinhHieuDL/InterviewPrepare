#pragma once

#include "fwStateMachine.h"

typedef enum{
    STATE_INIT = 0,
    STATE_RUNNING,
    STATE_STOPPED,
    STATE_ERROR
} StateID;    

typedef enum {
    ACTION_ENTER_INIT = 0,
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
const STATE_EVENT_CONTROL pEventControlInitList[] = {
    {EVENT_START, STATE_RUNNING, 0},
    {EVENT_ERROR, STATE_ERROR, ACTION_WARNING_ERROR},    
};  

const STATE_EVENT_CONTROL pEventControlRunningList[] = {
    {EVENT_STOP, STATE_STOPPED, 0},
    {EVENT_ERROR, STATE_ERROR, ACTION_WARNING_ERROR},
};

const STATE_EVENT_CONTROL pEventControlStoppedList[] = {
    {EVENT_START, STATE_RUNNING, 0},
    {EVENT_ERROR, STATE_ERROR, ACTION_WARNING_ERROR},
    {EVENT_RESET, STATE_INIT, 0}
};

const STATE_EVENT_CONTROL pEventControlErrorList[] = {
    {EVENT_RESET, STATE_INIT, 0}
};

// Define the states with their respective event control lists 
const STATE pTestStateList[] = {
    {STATE_INIT, ACTION_ENTER_INIT, ACTION_EXIT_INIT, (STATE_EVENT_CONTROL*)pEventControlInitList},
    {STATE_RUNNING, ACTION_ENTER_RUNNING, ACTION_EXIT_RUNNING, (STATE_EVENT_CONTROL*)pEventControlRunningList},
    {STATE_STOPPED, ACTION_ENTER_STOPPED, ACTION_EXIT_STOPPED, (STATE_EVENT_CONTROL*)pEventControlStoppedList},
    {STATE_ERROR, ACTION_ENTER_ERROR, ACTION_EXIT_ERROR, (STATE_EVENT_CONTROL*)pEventControlErrorList}
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