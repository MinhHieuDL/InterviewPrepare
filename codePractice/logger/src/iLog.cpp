#include <chrono>
#include "iLog.h"

uint64_t iLog::getTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();

    auto miliseconds = std::chrono::duration_cast<std::chrono::microseconds>(
                        duration)
                        .count();
    return (uint64_t)miliseconds;
}