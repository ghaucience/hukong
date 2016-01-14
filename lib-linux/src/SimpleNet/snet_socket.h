///
/// @author Kevin Lynx
/// @date 5.22.2008
/// @modify by lbzhung
///

#ifndef _SNET_SOCKET_H_
#define _SNET_SOCKET_H_

#include "snet_config.h"
#include "snet_address.h"
#include "snet_tool.h"


namespace lib_linux 
{
	// socket, wrap the basic socket behavior.
	class Socket
	{
	public:
		// constructor
		Socket()
		{
			_socket = INVALID_SOCKET;
		}
		
		// destructor
		virtual ~Socket()
		{
            release();
		}

        // check whether the socket is valid
        bool is_valid()
        {
            return _socket != INVALID_SOCKET;
        }

        // get local address
        Address get_local_addr()
        {
            ASSERT( is_valid() );

            sockaddr_in addr ;
            SOCKET_LEN_TYPE len = sizeof( addr );

            int ret = ::getsockname( _socket, (sockaddr*) &addr, &len );
            if( ret != SOCKET_ERROR )
            {
                return Address(addr);
            }
            else
            {
                //return invalid address
                return Address();
            }
        }

        static Address get_host_name(const char *pName)
        {
            struct hostent *pHost = ::gethostbyname(pName);
            if (pHost == NULL)
            {
                //OnError(WSAGetLastError(), "gethostbyname error");
                return Address();
            }

            if (pHost->h_addrtype != AF_INET)
            {
                //OnError(WSAGetLastError(), "gethostbyname return ipv6 address, not support");
                return Address();
            }

            char **pPtr = pHost->h_addr_list;
            in_addr_t ip_addr = *(in_addr_t *)(*pPtr);

            sockaddr_in addr;
            memset( &addr, 0, sizeof(addr));
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = ip_addr;

            return Address(addr);
        }

        // get the peer(remote) address.
        const Address & get_peer_addr()
        { 
            ASSERT( is_valid() );
            return _peer_addr;
        }

        // query the socket handle
        SOCKET query_socket() const
        {
            return _socket;
        }

        // create the socket
        bool create()
        {
            ASSERT( !is_valid() );
            _socket = ::socket( AF_INET, SOCK_STREAM, 0 );

            if (_socket == INVALID_SOCKET)
            {
                OnError(WSAGetLastError(), "Create listen socket error");
                return false;
            }

            return true;
        }

        // bind local address ( ip and port )
        bool bind( const Address &address )
        {
            ASSERT( is_valid() );
            int ret;

#if !defined (_WIN32)
            // reuse port when port in TIME_WAIT
            // TODO: Test in windows
            int turniton = 1;
            ret = ::setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &turniton, sizeof(turniton));
            if (ret == SOCKET_ERROR)
            {
                OnError(WSAGetLastError(), "Set TCP reuseaddr error");
                return false;
            }
#endif

            sockaddr_in addr = address;
            int len = sizeof(addr);
            ret = ::bind( _socket, (sockaddr*) &addr, len);	
            if (ret == SOCKET_ERROR)
            {
                OnError(WSAGetLastError(), "Bind port error");
                release();
                return false;
            }

            return true;
        }

        bool bind( const std::string &ip, unsigned short port )
        {
            return bind( Address( ip, port ) );
        }

        bool connect(const Address &addr)
        {
            ASSERT(is_valid());

            sockaddr_in address;
            address = addr;

            int ret = ::connect(_socket, (sockaddr*)&address, sizeof(address));
            if (ret == SOCKET_ERROR)
            {
                // no blocking socket will return WOULDBLOCK
                int err = WSAGetLastError();
#if defined(_WIN32)
                if (err != WSAEWOULDBLOCK)
#else
                if (err != EINPROGRESS)
#endif
                {
                    OnError(err, "connect error");
                    return false;
                }
            }

            _peer_addr = addr;
            return true;
        }

        bool connect_nb(const Address &addr)
        {
            ASSERT(is_valid());

            // set session socket to non-blocking
            SetBlocking(false);
            return connect(addr);
        }

        bool listen(int backlog)
        {
            ASSERT( is_valid() );

            int ret = ::listen(_socket, backlog);
            if (ret == SOCKET_ERROR)
            {
                OnError(WSAGetLastError(), "listen port error, 端口是否被占用");
                release();
                return false;
            }
            return true;
        }

        bool accept(Socket &listen)
        {
            ASSERT( !is_valid());

            sockaddr_in addr;
            SOCKET_LEN_TYPE len = sizeof( addr );

            _socket = ::accept(listen.query_socket(), (sockaddr*) &addr, &len );
            if (_socket == INVALID_SOCKET)
            {
                OnError(WSAGetLastError(), "accept socket error");
                return false;
            }

            _peer_addr = addr;
            return true;
        }

        // release the socket.
        void release()
        {
            if (is_valid())
            {
                snet_closesocket(_socket);
                _socket = INVALID_SOCKET;
            }
        }

        // set keepalive timer, default is 1 hour
        bool SetKeepAlive(unsigned long ulIntervalTime = 60*60*1000)
        {
            ASSERT(is_valid());

            // open keepalivetimer
            unsigned int dwKeepTimer = 1;
            ::setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, (char *)&dwKeepTimer, sizeof(dwKeepTimer));

#if defined(_WIN32)
            // set keepalive timer
            tcp_keepalive time;
            time.onoff = 1;
            time.keepalivetime = ulIntervalTime;
            // interval of ack
            time.keepaliveinterval = 1000;

            unsigned int dwkeepliveLen = sizeof(tcp_keepalive);

            // setting
            unsigned int dwReturn;
            int ret = ::WSAIoctl(_socket, SIO_KEEPALIVE_VALS, &time, dwkeepliveLen, NULL, 0, &dwReturn, NULL, NULL);
            if (ret == SOCKET_ERROR)
            {
                OnError(WSAGetLastError(), "Set TCP KeepAliveTimer error");
                return false;
            }
#else
            // add linux tcp timealive
            int keep_idle = ulIntervalTime / 1000;
            int keep_interval = 1;
            int keep_count = 5;

            int ret = ::setsockopt(_socket, SOL_TCP, TCP_KEEPIDLE, (void *)&keep_idle, sizeof(keep_idle));
            if (ret == SOCKET_ERROR)
            {
                OnError(WSAGetLastError(), "Set TCP KeepAliveTimer error");
                return false;
            }
            ret = ::setsockopt(_socket, SOL_TCP, TCP_KEEPINTVL, (void *)&keep_interval, sizeof(keep_interval));
            if (ret == SOCKET_ERROR)
            {
                OnError(WSAGetLastError(), "Set TCP KeepAliveTimer error");
                return false;
            }
            ret = ::setsockopt(_socket, SOL_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count));
            if (ret == SOCKET_ERROR)
            {
                OnError(WSAGetLastError(), "Set TCP KeepAliveTimer error");
                return false;
            }
#endif 
            return true;
        }

        // set socket block
        bool SetBlocking(bool bSet)
        {
            u_long arg = bSet ? 0 : 1;
#if defined(_WIN32)
            int rc = ::ioctlsocket(_socket, FIONBIO, &arg);
#else
            int rc = ::ioctl(_socket, FIONBIO, &arg);
            //int rc = fcntl(_socket, F_SETFL, O_NONBLOCK);
#endif
            return rc != 0;
        }

        int socket_recv(char *buffer, int nLen)
        {
            ASSERT(is_valid());

            int ret;
            int error;
#if defined(_WIN32)
            ret = ::recv(_socket, buffer, nLen, 0);
            error = WSAGetLastError();
#else
            do
            {
                ret = ::recv(_socket, buffer, nLen, 0);
                error = WSAGetLastError();
            }
            while ((ret == -1) && (errno == EINTR));
#endif

            if (error == -1)
            {
                // EWOULDBLOCK\WSAEWOULDBLOCK will return if socket is nonblocking
#if defined(_WIN32)
                if (errno != WSAEWOULDBLOCK)
#else
                if (errno != EWOULDBLOCK)
#endif
                {
                    OnError(WSAGetLastError(), "socket_recv return error");
                    return 0;
                }
            }

            return ret;
        }


        int socket_send(const char *buffer, int nLen)
        {
            ASSERT(is_valid());

            int ret;
            int error;

#if defined(_WIN32)
            ret = ::send(_socket, buffer, nLen, 0);
            error = WSAGetLastError();
#else
            do
            {
                ret = ::send(_socket, buffer, nLen, 0);
                error = WSAGetLastError();
            }
            while ((ret == -1) && (errno == EINTR));
#endif

            if (error == -1)
            {
                // EWOULDBLOCK\WSAEWOULDBLOCK will return if socket is nonblocking
#if defined(_WIN32)
                if (errno != WSAEWOULDBLOCK)
#else
                if (errno != EWOULDBLOCK)
#endif
                {
                    OnError(WSAGetLastError(), "socket_send return error");
                    return 0;
                }
            }

            return ret;
        }

        bool SetSendTimeout(int seconds)
        {
            ASSERT(is_valid());

            struct timeval timeo;
            socklen_t len = sizeof(timeo);
            timeo.tv_sec = seconds;
            timeo.tv_usec = 0;

            return ::setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, &timeo, len) == 0;
        }

        bool SetRecvTimeout(int seconds)
        {
            ASSERT(is_valid());

            struct timeval timeo;
            socklen_t len = sizeof(timeo);
            timeo.tv_sec = seconds;
            timeo.tv_usec = 0;

            return ::setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &timeo, len) == 0;
        }

    protected:
        // error event handle
        virtual void OnError(int nErrorCode, const char *pStr)
        {
            SIMPLE_NET_DEBUG("%s", pStr);
        }

    private:
        // the kernel socket handle
        SOCKET _socket;

        // peer address
        Address _peer_addr;
    };
}

#endif
