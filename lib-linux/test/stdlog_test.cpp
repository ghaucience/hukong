#include <iostream>
#include "lib_linux_config.h"
#include "stdlog.h"
#include "stdnetlog.h"
using namespace std;
using namespace lib_linux;

int main(int argc, char **argv)
{
    try
    {
        //StdLogNetHandler handler("127.0.0.1", 8080);

        StdLogOutHandler handler;
        ColorDecoratorHandler colorHandler(&handler);
        StdLog log(&colorHandler);

        log.SetLevel(LOG_LEVEL_DEBUG);
        log.Debug("%s %d\n", "debug", 1);
        log.Info("%s %d\n", "info", 2);
        log.Warning("%s %d\n", "warning", 3);
        log.Error("%s %d\n", "error", 4);

        SET_LOG(FLAG_SYSLOG, ERROR);
        DEBUG("%s %d", "debug", 1);
        INFO("%s %d", "info", 2);
        WARNING("%s %d", "warning", 3);
        ERROR("%s %d", "error", 4);
    }
    catch (const char *pStr)
    {
        cout<<pStr<<endl;
        return -1;
    }
    return 0;
}
