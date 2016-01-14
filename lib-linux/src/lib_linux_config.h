#ifndef __LIB_LINUX_CONFIG__
#define __LIB_LINUX_CONFIG__

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include "stdlog.h"

namespace lib_linux
{
    enum{
        FLAG_COLOR = 0x02,
        FLAG_SYSLOG = 0x04,
        FLAG_NET = 0x08,
        FLAG_CON = 0x10,
    };

    void SetLogger(int flag=FLAG_CON|FLAG_COLOR, int level=lib_linux::LOG_LEVEL_DEBUG);
    StdLog &GetCurLogger();
}

#define SET_LOG(flag, level)                   lib_linux::SetLogger(flag, lib_linux::LOG_LEVEL_##level)
#define DEBUG(format, args...)                 lib_linux::GetCurLogger().Debug(format"\n", ##args)
#define DEBUG_HEX(pData, nLen)                 lib_linux::GetCurLogger().Debug_HEX(pData, nLen);
#define DEBUG_HEX_PRE(pData, nLen, prefix)     lib_linux::GetCurLogger().Debug_HEX(pData, nLen, prefix, "");
#define DEBUG_HEX_MARK(pData, nLen)            lib_linux::GetCurLogger().Debug_HEX(pData, nLen, "\033[33m", "\033[0m");
#define INFO(format, args...)                  lib_linux::GetCurLogger().Info(format"\n", ##args)
#define WARNING(format, args...)               lib_linux::GetCurLogger().Warning(format"\n", ##args)
#define ERROR(format, args...)                 lib_linux::GetCurLogger().Error(format"\n", ##args)


#endif
