#include <iostream>
#include "stack.h"

Stack::Stack(std::size_t siSize):
    m_siStackSize{siSize},
    m_iStackTop{-1},
    m_pStackPool{siSize ? std::make_unique<int[]>(siSize) : nullptr}
{
    if(siSize == 0)
        throw exStackInvalidArg{};
};

Stack::~Stack()
{
    std::cout << "Stack destructed" << std::endl;
};

void Stack::push(int iData)
{
    if(m_iStackTop + 1 >= static_cast<int>(m_siStackSize))
        throw exStackFull{};
    m_pStackPool[++m_iStackTop] = iData;
};

int Stack::pop()
{
    if(m_iStackTop < 0)
        throw exStackEmpty{};
    return m_pStackPool[m_iStackTop--];
};