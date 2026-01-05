#include "logQueue.h"

void logQueue::shutdown()
{
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    m_bStopped = true;
    lock.unlock();
    m_cv.notify_all();
}

bool logQueue::pushMsg(std::string logMsg)
{
    bool bPushed = false;
    
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        if(!m_bStopped)
        {
            m_logQueue.push(std::move(logMsg));
            bPushed = true;
        }
    }
    
    if(bPushed)
    {
        m_cv.notify_one();
    }

    return bPushed;
}

bool logQueue::wait_and_pop(std::string& msg)
{
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    
    // Wait until there is data to comsume or shutdown requested
    m_cv.wait(lock, [this] {
        return !m_logQueue.empty() || m_bStopped;
    });
    
    // if producer shutdown and no more messages exits, signal caller to exit
    if(m_logQueue.empty()) return false;

    msg = std::move(m_logQueue.front());
    m_logQueue.pop();
    return true;
}