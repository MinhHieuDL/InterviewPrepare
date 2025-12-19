#include "fLog.h"

int main(void)
{
    iLog* pLogger = new fLog("log.txt");
    pLogger->log(LOG_LVL::INFO, "hello world");
    return 0;
}