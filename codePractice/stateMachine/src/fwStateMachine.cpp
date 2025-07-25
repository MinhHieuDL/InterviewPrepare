#include "fwStateMachine.h"
#include <iostream>

using namespace std;

CStateMachine::CStateMachine(/* args */) : 
    m_CurrentState{0},
    m_StateList{}
{ };

CStateMachine::~CStateMachine()
{
    cout << "State Machine removed" << endl;
}


void CStateMachine::Init(vector<STATE> StateList, unsigned char ucFirstState){
    // load the state list
    m_StateList = StateList;
    
    //load the first state
    LoadStateFromStateID(ucFirstState, m_CurrentState);

    // Perform the enter init state action
    CallMapAction(m_CurrentState.m_enterActionID);
}

void CStateMachine::LoadStateFromStateID(unsigned char ucState, STATE& refState){
    for(const auto& state : m_StateList){
        if(state.m_stateID == ucState){
            refState = state;
            return;
        }
    }
    cout << "Failed to load the State from state ID" << endl;
}

void CStateMachine::HandleEvt(unsigned char ucEvt) {
    vector<STATE_EVENT_CONTROL> EvtControlList = m_CurrentState.m_EventControlStateList;
    for(const auto& evtControl : EvtControlList)
    {
        if(evtControl.m_eventID == ucEvt)
        {
            CallMapAction(evtControl.m_actionID);
            
            // check if need to switch state
            if(evtControl.m_nextState != m_CurrentState.m_stateID)
            {
                // exit current state
                CallMapAction(m_CurrentState.m_exitActionID);

                // Load new state
                LoadStateFromStateID(evtControl.m_nextState, m_CurrentState);

                // do enter new state action
                CallMapAction(m_CurrentState.m_enterActionID);
            }
            break;
        }
    }
}
