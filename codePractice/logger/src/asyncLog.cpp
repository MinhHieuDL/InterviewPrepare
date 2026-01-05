#include "asyncLog.h"

AsyncLog::AsyncLog(iLog* pLogSynk){
    m_pLogSynk = std::unique_ptr<iLog>{pLogSynk};
}

AsyncLog::~AsyncLog(){
    m_logQueue.shutdown();
}

uint64_t AsyncLog::getTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    auto miliseconds = std::chrono::duration_cast<std::chrono::microseconds>(
                        duration)
                        .count();
    return (uint64_t)miliseconds;
}

void AsyncLog::log(LOG_LVL eLevel,const std::string &msg)
{
    std::string strLogMsg = '[' + toStringHelper(eLevel) + ']' + ' ' + 
                            '[' + getTimeStamp() + ']' + ' ' +
                            msg;

    m_logQueue.pushMsg(strLogMsg);
}

void AsyncLog::start()
{
    std::string logToWrite;

    while(m_logQueue.wait_and_pop(logToWrite))
    {
        m_pLogSynk->log(logToWrite);
    }
}