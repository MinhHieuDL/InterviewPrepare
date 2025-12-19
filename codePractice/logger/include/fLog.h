#pragma once

#include "iLog.h"
#include <fstream>

class fLog : public iLog {
    private:
        std::ofstream m_ofStream;
    
    public: 
        fLog(std::string sFileName);
        ~fLog();

        void log(LOG_LVL eLevel, const std::string &msg) override;

};