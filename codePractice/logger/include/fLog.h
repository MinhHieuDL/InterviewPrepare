#include "iLog.h"
#include <fstream>

class fLog : public iLog {
    private:
        std::ofstream m_ofStream;
    
    public: 
        fLog(std::string sFileName);
        ~fLog();

        void log(LOG_LVL eLevel, std::string msg);

};