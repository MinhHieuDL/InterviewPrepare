#pragma once

class Stack
{
    private:
        int m_iStackSize;
        int m_iStackTop;
        int* m_pStackPool;
    public:
        Stack(const unsigned uiStackSize);
        ~Stack();
        void push(int iData);
        int pop();
};