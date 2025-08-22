#include "stack.h"
#include <iostream>

using namespace std;

Stack::Stack(const unsigned uiStackSize):
    m_iStackSize{uiStackSize},
    m_iStackTop{-1}
{
    cout << "uiStackSize: " << uiStackSize << endl;
    m_pStackPool = new int[uiStackSize];
};

Stack::~Stack()
{
    cout << "delete stack pool" << endl;
    delete[] m_pStackPool;
};

void Stack::push(int iData)
{
    m_pStackPool[++m_iStackTop] = iData;
};

int Stack::pop()
{
    return m_pStackPool[m_iStackTop--];
};