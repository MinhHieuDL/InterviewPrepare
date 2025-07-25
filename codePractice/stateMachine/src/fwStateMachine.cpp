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
    for(unsigned i = 0; i < m_StateList.size(); i++){
        if(m_StateList[i].m_stateID == ucState){
            refState = m_StateList[i];
            return;
        }
    }
    cout << "Failed to load the State from state ID" << endl;
}

void CStateMachine::HandleEvt(unsigned char ucEvt) {
    vector<STATE_EVENT_CONTROL> EvtControlList = m_CurrentState.m_EventControlStateList;
    for(unsigned i = 0; i < EvtControlList.size(); i++)
    {
        if(EvtControlList[i].m_eventID == ucEvt)
        {
            CallMapAction(EvtControlList[i].m_actionID);
            
            // check if need to switch state
            if(EvtControlList[i].m_nextState != m_CurrentState.m_stateID)
            {
                // exit current state
                CallMapAction(m_CurrentState.m_exitActionID);

                // Load new state
                LoadStateFromStateID(EvtControlList[i].m_nextState, m_CurrentState);

                // do enter new state action
                CallMapAction(m_CurrentState.m_enterActionID);
            }
            break;
        }
    }
}
