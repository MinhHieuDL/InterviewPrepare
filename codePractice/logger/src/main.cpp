#include "fLog.h"
#include <memory>

int main(void)
{
    std::unique_ptr<iLog> pLogger = std::make_unique<fLog>("log.txt");
    pLogger->log(LOG_LVL::INFO, "hello world");
    return 0;
}