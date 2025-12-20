#include <iostream>
#include "fLog.h"

fLog::fLog(std::string sFileName) :
    m_ofStream(sFileName, std::ios::app)
{ };

fLog::~fLog()
{
    m_ofStream.close();
}

void fLog::log(LOG_LVL eLevel, const std::string &msg)
{
    if(m_ofStream.is_open())
    {
        m_ofStream << "[" << toStringHelper(eLevel) << "] " 
                   << "[" << getTimeStamp() << "] "
                   << msg << std::endl;
    }
    else {
        std::cerr << "Error: Unable to open log file" << std::endl;
    }
}