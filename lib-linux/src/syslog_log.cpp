#include "syslog_log.h"
#include <syslog.h>

namespace lib_linux
{
    SyslogHandler::SyslogHandler(const char *pstrIdent)
    {
        ::openlog(pstrIdent, 0, LOG_USER);
    }

    SyslogHandler::~SyslogHandler()
    {
        ::closelog();
    }

    void SyslogHandler::Write(int level, const char *format, va_list arg)
    {
        int priority = 0;
        switch (level)
        {
            case LOG_LEVEL_ERROR:
                priority = LOG_ERR;
                break;
            case LOG_LEVEL_WARNING:
                priority = LOG_WARNING;
                break;
            case LOG_LEVEL_INFO:
                priority = LOG_INFO;
                break;
            case LOG_LEVEL_DEBUG:
                priority = LOG_DEBUG;
                break;
            default:
                break;
        }

        vsyslog(priority, format, arg);
    }
}
