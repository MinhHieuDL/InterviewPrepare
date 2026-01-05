#include "asyncLog.h"
#include "fLog.h"
#include <vector>
#include <functional>

void single_thread_test()
{
    AsyncLog testLog(std::make_unique<fLog>("logA.txt"));
    
    // start worker thread
    testLog.start();

    // testlog
    for(int i = 0; i < 100; i++)
    {
        std::string logMsg = "count " + std::to_string(i);
        testLog.log(LOG_LVL::INFO, logMsg);
    }
}

void producerLog(unsigned uiThreadID, AsyncLog& logger)
{
    // testlog
    for(int i = 0; i < 100; i++)
    {
        std::string logMsg;
        logMsg += "[";
        logMsg += std::to_string(uiThreadID);
        logMsg += "] ";
        logMsg += "count ";
        logMsg += std::to_string(i);

        logger.log(LOG_LVL::INFO, logMsg);
    }
}


void multiple_thread_test()
{
    AsyncLog testLog(std::make_unique<fLog>("logA.txt"));
    // start worker thread
    testLog.start();

    const unsigned ui_numThreads = 4;
    std::vector<std::thread> threadList;
    threadList.reserve(ui_numThreads);

    for(unsigned uiTid = 0; uiTid < ui_numThreads; uiTid++)
    {
        threadList.emplace_back(producerLog, uiTid, std::ref(testLog));
    }

    for(auto& thread: threadList) thread.join();

}

int main(void)
{
    multiple_thread_test();
    return 0;
}