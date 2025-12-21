#include <iostream>
#include "fLog.h"

fLog::fLog(const std::string& sFileName) :
    m_ofStream(sFileName, std::ios::app)
{ };

void fLog::log(LOG_LVL eLevel, const std::string &msg)
{
    if(m_ofStream.is_open())
    {
        m_ofStream << "[" << toStringHelper(eLevel) << "] " 
                   << "[" << getTimeStamp() << "] "
                   << msg << '\n';
    }
    else {
        std::cerr << "Error: Unable to open log file" << '\n';
    }
}