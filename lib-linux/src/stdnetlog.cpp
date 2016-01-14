#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include "stdnetlog.h"

namespace lib_linux
{
    StdLogNetHandler::StdLogNetHandler(const char *pstrIP, unsigned short port)
    {
        m_socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket == -1)
        {
            throw "StdLogNetHandler create socket fail";
        }

        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr(pstrIP);
        address.sin_port = htons(port);

        int ret = ::connect(m_socket, (sockaddr*)&address, sizeof(address));
        if (ret == -1)
        {
            throw "StdLogNetHandler connect fail";
        }
    }

    StdLogNetHandler::~StdLogNetHandler()
    {
        ::close(m_socket);
        m_socket = -1;
    }

    void StdLogNetHandler::Write(int level, const char *format, va_list arg)
    {
        assert(m_socket != -1);
        char buffer[10*1024];
        int nLen = vsnprintf(buffer, sizeof(buffer), format, arg);
        if (nLen > 0)
        {
            buffer[nLen] = '\0';

            int ret = -1;
            int error = -1;
            do
            {
                ret = ::send(m_socket, buffer, nLen, 0);
                error = errno;
            }
            while ((ret == -1) && (errno == EINTR));

            if (error == -1)
            {
                throw "StdLogNetHandler send fail";
            }
        }
    }
}
