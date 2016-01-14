#ifndef _SYSLOG_LOG_H_
#define _SYSLOG_LOG_H_
#include <cstdio>
#include "stdlog.h"

namespace lib_linux
{
    class SyslogHandler:public StdLogHandler
    {
        public:
            SyslogHandler(const char *pstrIdent=NULL);
            ~SyslogHandler();

            void Write(int level, const char *format, va_list arg);
    };
}

#endif
