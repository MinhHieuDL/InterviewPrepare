#include <thread>
#include "logQueue.h"

logQueue::logQueue(iLog* pLogConcrete) : m_pLog(pLogConcrete)
{}
        
void logQueue::pushMsg(const std::string logMsg)
{
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    m_logQueue.push(logMsg);
    lock.unlock();
}

void logQueue::writeLog()
{
    while(!m_logQueue.empty())
    {
        m_pLog->log(LOG_LVL::INFO, m_logQueue.front());
        m_logQueue.pop();
    }
}