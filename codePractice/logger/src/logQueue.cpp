#include "logQueue.h"
        
void logQueue::pushMsg(const std::string logMsg)
{
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    m_logQueue.push(logMsg);
    lock.unlock();
    m_cv.notify_one();
}

void logQueue::wait_and_pop(std::string& msg)
{
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    while(m_logQueue.empty())
        m_cv.wait(lock);
    msg = m_logQueue.front();
    m_logQueue.pop();
    lock.unlock();
}