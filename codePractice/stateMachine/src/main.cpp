#include "test.h"
#include <iostream>

using namespace std;

int main(void){
    CStateMachine* testSM = new Test();
    testSM->Init(TestStateList, STATE_INIT);
    unsigned char receivedEvt;
    while(true){
        cout << "enter event: ";
        cin >> receivedEvt;
        switch (receivedEvt)
        {
        case 's':
        {
            cout << "Received EVENT_START" << endl;
            testSM->HandleEvt(EVENT_START);
        }
        break;

        case 'p':
        {
            cout << "Received EVENT_STOP" << endl;
            testSM->HandleEvt(EVENT_STOP);
        }
        break;

        case 'e':
        {
            cout << "Received EVENT_ERROR" << endl;
            testSM->HandleEvt(EVENT_ERROR);
        }
        break;
        
        case 'r':
        {
            cout << "Received EVENT_RESET" << endl;
            testSM->HandleEvt(EVENT_RESET);
        }
        break;

        case 'o':
        {
            cout << "Exit app" << endl;
            delete testSM;
        }
        return 0;
        
        default:
            cout << "unknown event" << endl;
        break;
        }
    }
    
    return 0;
}