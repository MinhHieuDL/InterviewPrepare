#pragma once

#include <string>
#include "logHelper.h"

class iLog
{   
    public:
        iLog()=default;
        virtual ~iLog()=default;
        virtual void log(LOG_LVL eLevel,const std::string &msg)=0;

    protected:
        uint64_t getTimeStamp();
};