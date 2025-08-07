#pragma once

#include <vector>

struct STATE_EVENT_CONTROL{
    unsigned char m_eventID;
    unsigned char m_nextState;
    unsigned char m_actionID;
};

struct STATE{
    unsigned char m_stateID;
    unsigned char m_enterActionID;
    unsigned char m_exitActionID;
    std::vector<STATE_EVENT_CONTROL> m_EventControlStateList;
};

class CStateMachine {
    private:
        STATE m_CurrentState;
        std::vector<STATE> m_StateList;
        
        void LoadStateFromStateID(unsigned char ucState, STATE& refState);
    protected:
        virtual void CallMapAction(unsigned char ucActionID) = 0;
    
    public:
        CStateMachine(/* args */);
        virtual ~CStateMachine();
        
        void Init(std::vector<STATE> StateList, unsigned char ucFirstState);
        void HandleEvt(unsigned char ucEvt);
};

