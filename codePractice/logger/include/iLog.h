#pragma once

#include <string>
#include "logHelper.h"

class iLog
{   
    public:
        iLog()=default;
        virtual ~iLog()=default;
        virtual void log(const std::string &msg)=0;        
};