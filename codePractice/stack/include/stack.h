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