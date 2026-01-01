#pragma once

#include "iLog.h"
#include <fstream>

class fLog : public iLog {
    private:
        std::ofstream m_ofStream;
    
    public: 
        fLog(const std::string& sFileName);
        ~fLog() = default;

        // the fLog non-copyable
        fLog(const fLog&) = delete;
        fLog& operator=(const fLog&) = delete;
         
        // enble fLog movable
        fLog(fLog&&) noexcept = default;
        fLog& operator=(fLog&&) noexcept = default;

        void log(LOG_LVL eLevel, const std::string &msg) override;

};