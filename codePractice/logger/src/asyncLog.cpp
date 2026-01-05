#include <chrono>
#include <stdexcept>
#include "asyncLog.h"

AsyncLog::AsyncLog(std::unique_ptr<iLog> pLogSynk)
    : m_pSink(std::move(pLogSynk))
{
    if(!m_pSink)
        throw std::invalid_argument("AsyncLog sink is null");
}

AsyncLog::~AsyncLog(){
    stop();
}

uint64_t AsyncLog::getTimeStampMs()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto duration = now.time_since_epoch();

    auto miliseconds = duration_cast<std::chrono::milliseconds>(
                       duration)
                       .count();

    return (uint64_t)miliseconds;
}

bool AsyncLog::log(LOG_LVL eLevel,const std::string &msg)
{
    if(!m_bStarted.load()) return false;

    //format message to log
    std::string strLogMsg;
    strLogMsg += "[";
    strLogMsg += toStringHelper(eLevel);
    strLogMsg += "] ";
    strLogMsg += "[";
    strLogMsg += std::to_string(getTimeStampMs());
    strLogMsg += "] ";
    strLogMsg += msg;

    return m_logQueue.pushMsg(std::move(strLogMsg));
}

void AsyncLog::workerLoop()
{
    std::string strLog;
    while (m_logQueue.wait_and_pop(strLog))
    {
        m_pSink->log(strLog);
    }
}

void AsyncLog::start()
{
    bool b_Expected = false;
    if (!m_bStarted.compare_exchange_strong(b_Expected, true))
        return; // already started

    m_thrWorker = std::thread(&AsyncLog::workerLoop, this);
}

void AsyncLog::stop()
{
    if (!m_bStarted.exchange(false))
        return;

    m_logQueue.shutdown();

    if(m_thrWorker.joinable()) m_thrWorker.join();
}