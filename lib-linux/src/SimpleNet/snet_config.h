#ifndef _SNET_CONFIG_H_
#define _SNET_CONFIG_H_

#include <stdexcept>

#if defined(_WIN32)
    #include <winsock2.h>
    // include WSAIoctl function
    #include <mstcpip.h>

    #define SOCKET_LEN_TYPE int
    #define snet_closesocket(x) ::closesocket(x)
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <netdb.h> 
    #include <unistd.h>

    #include <errno.h>
    #include <string.h>
    #include <stdarg.h>

    #define SOCKET int
    #define SOCKET_LEN_TYPE socklen_t
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR   (-1)
    #define WSAGetLastError() errno
    #define snet_closesocket(x) ::close(x)
#endif

#include <assert.h>
#ifndef ASSERT
    #define ASSERT(x) assert(x)
#endif

#if defined(_WIN32)
    #ifdef _DEBUG
        #define SIMPLE_NET_DEBUG(x)  ::OutputDebugString(x)
    #else
    #define SIMPLE_NET_DEBUG(x)
    #endif
#else
    #ifdef _DEBUG
        #include <cstdio>
        #define SIMPLE_NET_DEBUG(format, args...) printf("[SimpleNet DEBUG] [%s] ", strerror(errno));printf(format, ##args);printf("\n");
    #else
        #define SIMPLE_NET_DEBUG(format, args...) 
    #endif
#endif

#endif
