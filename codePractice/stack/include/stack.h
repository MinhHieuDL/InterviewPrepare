#pragma once

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
        class exStackFull{};
        class exStackEmpty{};
};