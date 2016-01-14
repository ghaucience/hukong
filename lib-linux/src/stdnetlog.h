#ifndef _STDNETLOG_H_
#define _STDNETLOG_H_
#include "stdlog.h"

namespace lib_linux
{
    class StdLogNetHandler:public StdLogHandler
    {
        public:
            StdLogNetHandler(const char *pstrIP, unsigned short port);
            ~StdLogNetHandler();

            void Write(int level, const char *format, va_list arg);
        private:
            int m_socket;
    };
}

#endif
