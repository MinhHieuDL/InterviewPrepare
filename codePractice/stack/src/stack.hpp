#pragma once
#include <stdexcept>
#include <memory>
template<typename T>
class Stack
{
    private:
        std::size_t m_siStackSize;
        int m_iStackTop;
        std::unique_ptr<T[]> m_pStackPool;
    public:
        // member fucntion
        explicit Stack (std::size_t siSize);
        ~Stack();
        void push(T tData);
        T pop();

        // class exception
        struct exStackFull : std::runtime_error{
            exStackFull() : std::runtime_error{"Stack is full!"} {} 
        } ;
        struct exStackEmpty : std::runtime_error{
            exStackEmpty() : std::runtime_error{"Stack is empty!"} {} 
        };
        struct exStackInvalidArg : std::runtime_error{
            exStackInvalidArg() : std::runtime_error{"Stack size must be > 0"} {}
        };
};

// ================= Implementation =================

template<typename T>
Stack<T>::Stack(std::size_t siSize):
    m_siStackSize{siSize},
    m_iStackTop{-1},
    m_pStackPool{siSize ? std::make_unique<T[]>(siSize) : nullptr}
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