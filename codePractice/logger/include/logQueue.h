#pragma once

#include <queue>
#include <string>
#include <mutex>

class logQueue{
    private:
        std::queue<std::string> m_logQueue;
        std::mutex m_QueueMutex;


    public:
        logQueue()=default;
        ~logQueue()=default;

        void pushMsg(const std::string logMsg);
};