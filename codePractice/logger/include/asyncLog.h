#include <memory>
#include "iLog.h"
#include "logQueue.h"

class AsyncLog
{
    private:
        std::unique_ptr<iLog> m_pLogSynk;
        logQueue m_logQueue;

    protected:
        uint64_t getTimeStamp();

    public:
        AsyncLog(iLog* pLogSynk);
        ~AsyncLog();

        void log(LOG_LVL eLevel,const std::string &msg);
        void start();
};