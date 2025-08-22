#include <iostream>
#include "stack.h"

Stack::Stack(const unsigned uiStackSize):
    m_iStackSize{uiStackSize},
    m_iStackTop{-1},
    m_pStackPool{std::make_unique<int[]>(uiStackSize)}
{
};

Stack::~Stack()
{
    std::cout << "Stack destructed" << std::endl;
};

void Stack::push(int iData)
{
    if(m_iStackTop + 1 >= m_iStackSize)
        throw exStackFull{};
    m_pStackPool[++m_iStackTop] = iData;
};

int Stack::pop()
{
    if(m_iStackTop < 0)
        throw exStackEmpty{};
    return m_pStackPool[m_iStackTop--];
};