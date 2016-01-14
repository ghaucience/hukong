#ifndef __RPC_CONFIG_H__
#define __RPC_CONFIG_H__

#define MAJOR   1
#define MINOR   9
#define RELEASE 34
#define EXTRA   dev

#define VER_YEAR    14
#define VER_MONTH   8
#define VER_DAY     20

#define _STR(x)  #x
#define STR(x)  _STR(x)
#define VERSION  STR(MAJOR)"."STR(MINOR)"."STR(RELEASE)" "STR(EXTRA)
#define VER_DATE STR(VER_YEAR)"."STR(VER_MONTH)"."STR(VER_DAY)

#define RPC_METHOD_CALL_RETURN_FAIL  -32500

#endif
