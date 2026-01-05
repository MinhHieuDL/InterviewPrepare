#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <cstdint>
#include "iLog.h"
#include "logQueue.h"

class AsyncLog
{
    private:
        std::unique_ptr<iLog> m_pSink;
        logQueue m_logQueue;
        
        std::atomic<bool> m_bStarted{false};
        std::thread m_thrWorker;

        static uint64_t getTimeStampMs();
        void workerLoop();
        void stop();
    
    public:
        AsyncLog(std::unique_ptr<iLog> pLogSynk);
        ~AsyncLog();

        AsyncLog(const AsyncLog&) = delete;
        AsyncLog& operator=(const AsyncLog&) = delete;

        bool log(LOG_LVL eLevel,const std::string &msg);
        
        void start();
};