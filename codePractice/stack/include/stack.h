#pragma once
#include <stdexcept>
class Stack
{
    private:
        unsigned m_iStackSize;
        int m_iStackTop;
        int* m_pStackPool;
    public:
        // member fucntion
        Stack(const unsigned uiStackSize);
        ~Stack();
        void push(int iData);
        int pop();

        // class exception
        struct exStackFull : std::runtime_error{
            exStackFull() : std::runtime_error{"Stack is full!"} {} 
        } ;
        struct exStackEmpty : std::runtime_error{
            exStackEmpty() : std::runtime_error{"Stack is empty!"} {} 
        };
};