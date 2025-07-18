typedef struct{
    unsigned char m_eventID;
    unsigned char m_nextState;
    unsigned char m_actionID;
} STATE_EVENT_CONTROL;

typedef struct{
    unsigned char m_stateID;
    unsigned char m_enterActionID;
    unsigned char m_exitActionID;
    STATE_EVENT_CONTROL* m_pEventControlStateList;
} STATE;

class CStateMachine {
    private:
        STATE m_CurrentState;
        STATE* m_pStateList;
        
        void LoadStateFromStateID(unsigned char ucState, STATE& refState);
    protected:
        virtual void CallMapAction(unsigned char ucActionID) = 0;
    
    public:
        void Init(STATE* pStateList, unsigned char ucFirstState);
        void HandleEvt(unsigned char ucEvt);
        CStateMachine(/* args */);
        ~CStateMachine();
};

