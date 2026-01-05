#include "asyncLog.h"
#include "fLog.h"

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

int main(void)
{
    
    return 0;
}