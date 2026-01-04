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
        bool m_bStopped{false};

    public:
        logQueue()=default;
        ~logQueue()=default;

        bool pushMsg(std::string logMsg);
        bool wait_and_pop(std::string& msg);
        void shutdown();
};