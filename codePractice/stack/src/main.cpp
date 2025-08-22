#include <iostream>
#include "stack.h"

using namespace std;

int main(void)
{
    try
    {
        Stack stackObj(3);
        stackObj.push(1);
        stackObj.push(2);
        stackObj.push(3);
        stackObj.push(4);
        stackObj.push(5);
        cout << "Stack pop: " << stackObj.pop() << endl;
        cout << "Stack pop: " << stackObj.pop() << endl;  
        cout << "Stack pop: " << stackObj.pop() << endl;
        cout << "Stack pop: " << stackObj.pop() << endl;
    }
    catch(const runtime_error& e)
    {
        cout << e.what() << endl;
    }

    cout << "Program done" << endl;
    return 0;
}