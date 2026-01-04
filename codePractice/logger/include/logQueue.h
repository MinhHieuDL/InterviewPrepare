#pragma once

#include <queue>
#include <string>
#include <mutex>
#include "iLog.h"

class logQueue{
    private:
        std::queue<std::string> m_logQueue;
        std::mutex m_QueueMutex;
        iLog* m_pLog;

    public:
        logQueue(iLog* pLogConcrete);
        ~logQueue()=default;

        void pushMsg(const std::string logMsg);
        void writeLog();
};