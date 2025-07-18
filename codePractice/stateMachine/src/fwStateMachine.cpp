#include "fwStateMachine.h"
#include <iostream>

using namespace std;

CStateMachine::CStateMachine(/* args */) : 
    m_CurrentState{0},
    m_pStateList{nullptr}
{ };

CStateMachine::~CStateMachine()
{
    cout << "State Machine removed" << endl;
}


void CStateMachine::Init(STATE* pStateList, unsigned char ucFirstState){
    // load the state list
    m_pStateList = pStateList;
    
    //load the first state
    LoadStateFromStateID(ucFirstState, m_CurrentState);

    // Perform the enter init state action
    CallMapAction(m_CurrentState.m_enterActionID);
}

void CStateMachine::LoadStateFromStateID(unsigned char ucState, STATE& refState){
    unsigned char ucSize = sizeof(m_pStateList)/sizeof(STATE);
    for(unsigned i = 0; i < ucSize; i++){
        if(m_pStateList[i].m_stateID == ucState){
            refState = m_pStateList[i];
            break;
        }
    }
    cout << "Failed to load the State from state ID" << endl;
}

void CStateMachine::HandleEvt(unsigned char ucEvt) {
    STATE_EVENT_CONTROL* pListEvtControl = m_CurrentState.m_pEventControlStateList;
    unsigned char ucSize = sizeof(pListEvtControl)/sizeof(STATE_EVENT_CONTROL);
    for(unsigned i = 0; i < ucSize; i++)
    {
        bool b_IsDone{false};
        if(pListEvtControl[i].m_eventID == ucEvt)
        {
            CallMapAction(pListEvtControl[i].m_actionID);
            b_IsDone = true;
        }

        // check if need to switch state
        if(pListEvtControl[i].m_nextState != m_CurrentState.m_stateID)
        {
            // exit current state
            CallMapAction(m_CurrentState.m_exitActionID);
            
            // Load new state
            LoadStateFromStateID(pListEvtControl[i].m_nextState, m_CurrentState);
            
            // do enter new state action
            CallMapAction(m_CurrentState.m_enterActionID);
        }
        
        if(b_IsDone) break;
    }
}
