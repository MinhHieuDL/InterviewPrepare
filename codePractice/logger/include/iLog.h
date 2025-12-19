#pragma once

#include <string>
#include <map>
#include <vector>
enum class LOG_LVL{DEBUG, WARN, ERROR, INFO};

const std::map<LOG_LVL, std::string> lvl_string= {
    {LOG_LVL::DEBUG , "DEBUG"},
    {LOG_LVL::WARN  , "WARN"},
    {LOG_LVL::ERROR , "ERROR"},
    {LOG_LVL::INFO  , "INFO"}
};

class iLog
{       
    public:
        iLog()=default;
        virtual ~iLog()=default;
        virtual void log(LOG_LVL eLevel,const std::string &msg)=0;
};