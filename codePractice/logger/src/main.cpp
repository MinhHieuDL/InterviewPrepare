#include "fLog.h"
#include <memory>

int main(void)
{
    std::unique_ptr<iLog> pLoggerA = std::make_unique<fLog>("logA.txt");
    std::unique_ptr<iLog> pLoggerB = std::make_unique<fLog>("logB.txt");
    pLoggerB = std::move(pLoggerA);
    pLoggerB->log(LOG_LVL::INFO, "hello world");
    return 0;
}