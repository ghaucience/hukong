#ifndef _STDLOG_H_
#define _STDLOG_H_
#include <cstdarg>

namespace lib_linux
{
    enum
    {
        LOG_LEVEL_NONE = -1,
        LOG_LEVEL_ERROR = 0,
        LOG_LEVEL_WARNING = 1,
        LOG_LEVEL_INFO = 2,
        LOG_LEVEL_DEBUG = 3,
    };

    class StdLogHandler
    {
        public:
            StdLogHandler();
            virtual ~StdLogHandler();
            virtual void Write(int level, const char *format, va_list arg) = 0;
            void WriteString(int level, const char *format, ...);
    };

    class StdLogOutHandler:public StdLogHandler
    {
        public:
            // output to console
            void Write(int level, const char *format, va_list arg);
    };

    class ColorDecoratorHandler:public StdLogHandler
    {
        public:
            ColorDecoratorHandler(StdLogHandler *pHandler=NULL);
            void SetHandler(StdLogHandler *pHandler);
            // color output to console
            void Write(int level, const char *format, va_list arg);
        private:
            StdLogHandler *m_pHandler;
    };

    class StdLog
    {
        public:
            StdLog(StdLogHandler *pHandler=NULL);
            virtual ~StdLog();

            void SetHandler(StdLogHandler *pHandler);
            StdLogHandler *GetHandler();
            void SetLevel(int level);

            void Debug_HEX(const char *pData, int nLen, const char *pstrPrefix=NULL, const char *pstrSuffix=NULL);
            void Debug(const char *format, ...);
            void Info(const char *format, ...);
            void Warning(const char *format, ...);
            void Error(const char *format, ...);

        protected:
            void Write(int level, const char *format, va_list arg);

        private:
            StdLogHandler *m_pHandler;

            // print level less or equal than this
            int m_level;
    };
}

#endif
