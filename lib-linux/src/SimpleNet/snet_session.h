///
/// @author Kevin Lynx
/// @date 5.22.2008
/// @modify by lbzhung
///

#ifndef _SNET_SESSION_H_
#define _SNET_SESSION_H_

#include "buffer.h"
#include "snet_config.h"
#include "snet_address.h"
#include "snet_socket.h"

namespace lib_linux
{
    //
    // ISession class represents a connection between server and client.
    // You cannot create this class yourself.The server/client will create it when
    // some connection is established..
    //
    class SessionManager;
    class ISession:public Socket
    {
        public:
            // receive data, this function only return the queued data to you.
            // @return the number of bytes copyed
            int recv( char *buf, int len );

            // send data, this function only copy the data you provide to the queue,
            // and send it later.
            // @return the number of bytes it copyed
            int send( const char *buf, int len );

            int getid();

        protected:
            // only SessionManager can operate the Session.
            friend class SessionManager;

            // hide constructor
            // @param addr the remote address
            ISession();

            // do_recv, recv from TCP stack.This function will recv data and queue the data.
            // @return the number bytes it received.
            int do_recv();

            // do_send, send the queued data in the buffer to the peer.
            // @return the number bytes it send.
            //int do_send();

            // get buffer directly
            Buffer& GetBuffer();

        protected:
            // read event
            virtual void OnRead(SessionManager *pSessionManager) =0;

            // connect event
            virtual void OnConnect(SessionManager *pSessionManager) =0;

            // disconnect event
            // 注意: SessionManager析构时会调用OnDisconnect函数
            // 所以不能把指针pSessionManager再转换为SessionManager的子类，因为子类已经不存在了。
            virtual void OnDisconnect(SessionManager *pSessionManager) =0;

            // error event handle
            virtual void OnError(SessionManager *pSessionManager, int nErrorCode, const char *pStr)
            {
                Socket::OnError(nErrorCode, pStr);
            }

        private:
            // buffer to queue the send data.
            //Buffer _sendbuf;
            // buffer to queue the recv data
            Buffer _recvbuf;

            // globe unique session id 
            static int _start_id;

            // session id
            int _id;
    };
}

#endif
