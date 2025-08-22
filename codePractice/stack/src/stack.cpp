#include <iostream>
#include "stack.h"

Stack::Stack(const unsigned uiStackSize):
    m_iStackSize{uiStackSize},
    m_iStackTop{0}
{
    m_pStackPool = new int[uiStackSize];
};

Stack::~Stack()
{
    std::cout << "delete stack pool" << std::endl;
    delete[] m_pStackPool;
    m_pStackPool = nullptr;
};

void Stack::push(int iData)
{
    if(m_iStackTop >= m_iStackSize)
        throw exStackFull{};
    m_pStackPool[m_iStackTop++] = iData;
};

int Stack::pop()
{
    if(m_iStackTop < 0)
        throw exStackEmpty{};
    return m_pStackPool[m_iStackTop--];
};