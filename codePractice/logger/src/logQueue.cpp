#include <thread>
#include "logQueue.h"
        
void logQueue::pushMsg(const std::string logMsg)
{
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    m_logQueue.push(logMsg);
    lock.unlock();
}
