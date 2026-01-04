#pragma once

#include <queue>
#include <string>
#include <mutex>
#include <condition_variable>

class logQueue{
    private:
        std::queue<std::string> m_logQueue;
        std::mutex m_QueueMutex;
        std::condition_variable m_cv;

    public:
        logQueue()=default;
        ~logQueue()=default;

        void pushMsg(const std::string logMsg);
        void wait_and_pop(std::string& msg);
};