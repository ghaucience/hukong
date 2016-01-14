#include "lib_linux_config.h"
#include "syslog_log.h"
#include "singleton.h"

namespace lib_linux
{
    void SetLogger(int flag, int level)
    {
        assert((flag & FLAG_CON) || (flag & FLAG_SYSLOG));
        StdLogHandler *pHandler = NULL;
        if (flag & FLAG_CON)
        {
            // stdout console
            pHandler = Singleton<StdLogOutHandler>::Instance();
        }
        else
        {
            // syslog
            pHandler = Singleton<SyslogHandler>::Instance();
        }

        if (flag & FLAG_COLOR)
        {
            Singleton<ColorDecoratorHandler>::Instance()->SetHandler(pHandler);
            pHandler = Singleton<ColorDecoratorHandler>::Instance();
        }
        Singleton<StdLog>::Instance()->SetLevel(level);
        Singleton<StdLog>::Instance()->SetHandler(pHandler);
    }

    StdLog &GetCurLogger()
    {
        return *Singleton<StdLog>::Instance();
    }
}
