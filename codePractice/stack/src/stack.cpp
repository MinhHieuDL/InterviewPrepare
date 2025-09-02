#include <iostream>
#include "stack.h"

template<typename T>
Stack<T>::Stack(std::size_t siSize):
    m_siStackSize{siSize},
    m_iStackTop{-1},
    m_pStackPool{siSize ? std::make_unique<int[]>(siSize) : nullptr}
{
    if(siSize == 0)
        throw exStackInvalidArg{};
};

template<typename T>
Stack<T>::~Stack()
{
    std::cout << "Stack destructed" << std::endl;
};

template<typename T>
void Stack<T>::push(T tData)
{
    if(m_iStackTop + 1 >= static_cast<int>(m_siStackSize))
        throw exStackFull{};
    m_pStackPool[++m_iStackTop] = tData;
};

template<typename T>
T Stack<T>::pop()
{
    if(m_iStackTop < 0)
        throw exStackEmpty{};
    return m_pStackPool[m_iStackTop--];
};