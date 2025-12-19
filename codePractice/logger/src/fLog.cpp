#include <iostream>
#include "fLog.h"

fLog::fLog(std::string sFileName) :
    m_ofStream(sFileName, std::ios::app)
{ };

fLog::~fLog()
{
    m_ofStream.close();
}

void fLog::log(LOG_LVL eLevel, std::string msg)
{
    if(m_ofStream.is_open())
    {
        m_ofStream << "[" << lvl_string.at(eLevel) << "] " << msg << "\n";
    }
    else {
        std::cout << "Error: Unable to open log file" << std::endl;
    }
}