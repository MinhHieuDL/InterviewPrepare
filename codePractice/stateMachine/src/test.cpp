#include "test.h"
#include <iostream>

using namespace std;

void Test::EnterInitAction(){
    cout << "Entering Init State" << endl;
}

void Test::ExitInitAction(){
    cout << "Exiting Init State" << endl;
}

void Test::EnterRunningAction(){
    cout << "Entering Running State" << endl;
}

void Test::ExitRunningAction(){
    cout << "Exiting Running State" << endl;
}

void Test::EnterStoppedAction(){
    cout << "Entering Stopped State" << endl;
}

void Test::ExitStoppedAction(){
    cout << "Exiting Stopped State" << endl;
}

void Test::EnterErrorAction(){
    cout << "Entering Error State" << endl;
}

void Test::ExitErrorAction(){
    cout << "Exiting Error State" << endl;
}

void Test::WarningErrorAction(){
    cout << "Warning: An error has occurred!" << endl;
}

void Test::ResetAction(){
    cout << "Resetting State Machine" << endl;
}

void Test::CallMapAction(unsigned char ucActionID){
    switch(ucActionID){
        case ACTION_ENTER_INIT : return EnterInitAction();
        case ACTION_EXIT_INIT  : return ExitInitAction();
        case ACTION_ENTER_RUNNING : return EnterRunningAction();
        case ACTION_EXIT_RUNNING : return ExitRunningAction();
        case ACTION_ENTER_STOPPED : return EnterStoppedAction();
        case ACTION_EXIT_STOPPED : return ExitStoppedAction();
        case ACTION_ENTER_ERROR : return EnterErrorAction();
        case ACTION_EXIT_ERROR: return ExitErrorAction();
        case ACTION_WARNING_ERROR: return WarningErrorAction();
        case ACTION_RESET: return ResetAction();
        case NO_ACTION: return;
        default : { cout << "not find action" << endl; return;}
    }        
}