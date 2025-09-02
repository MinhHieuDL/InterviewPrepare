#include <iostream>
#include "stack.hpp"

using namespace std;

int main(void)
{
    try
    {
        Stack<int> stackObj(3);
        stackObj.push(1);
        stackObj.push(2);
        stackObj.push(3);
        stackObj.push(4); // comment out this 2 lines to test pop exception 
        stackObj.push(5); // comment out this 2 lines to test pop exception
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