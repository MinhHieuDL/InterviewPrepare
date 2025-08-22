#include <iostream>
#include "stack.h"

using namespace std;

int main(void)
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
    
    return 0;
}