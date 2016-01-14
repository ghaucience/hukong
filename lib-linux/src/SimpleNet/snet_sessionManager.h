#ifndef _SNET_SERVER_H_
#define _SNET_SERVER_H_

#include "snet_socket.h"
#include "snet_fdset.h"
#include "snet_session.h"
#include "snet_sessionFactory.h"

#include <map>
#include <list>

namespace lib_linux
{
    class SessionManager
    {
        public:
            // session container
            // on linux SOCKET value can duplicate, so don't use to session key.
            // TODO: change SOCKET to unique id
            typedef std::map<int, ISession*> SessionMap;
            typedef std::list<ISession *> SessionList;

        public:	
            // constructor
            SessionManager(int nMaxSession);

            // destructor
            virtual ~SessionManager();

            // startup server
            bool StartupServer(SessionFactory *pFactory, const char *pstrIP, unsigned short usPort);

            // connect to the server, if connectes ok, it will create a session.
            ISession *Connect(SessionFactory &factory, const Address &remote_addr, bool bBlocking = true);

            // poll the socket status, this function will accept new connection and poll every
            // session's status.
			// This function is non-blocking, and you must call it in a loop.
			// 这个函数是非阻塞的而且轮询的，所以必须在死循环中调用，一般在定时消息中调用，定时10ms
            int Poll(struct timeval &timeout);

            // shutdown, disconnect all the sessions and free socket resources
            void Shutdown();

            void Broadcast(const char *buf, int len, const ISession *pExclude=NULL);

            // get all sessions
            SessionList GetSessionList(void);

            // get count of sessions
            int GetSessionCount(void);

            // get count of waitting nonblocking sessions
            int GetWaitingNBSessionCount(void);

            // check the server status
            bool IsRunning(void);

            // put in list and later delete
            void Delete(ISession *pSession);

            // delete session directly, use by connect non-block mode
            void DeleteDirect(ISession *pSession);
        protected:

            // create a new session and save the session in the list.
            bool NewSession();

            // setup check fd set
            void SetupFdset(void);

            // SessionMap convert to SessionList
            SessionList SessionMapToList(SessionMap &map);

        private:
            // listen socket
            Socket _listen;

            // session list
            SessionMap _sessions;

            // non-blocking session list for connect
            SessionMap _nbconnect_sessions;

            // delete list
            SessionMap _delete_sessions;

			// 服务器session factory用于StartupServer
            SessionFactory *m_pFactoryServSession;

            // max session also max FD_SET size
            int  _max_fd;

            // read fd set
            Fdset _read_set;

            // write fd set
            Fdset _write_set;

            // error fd set
            Fdset _error_set;

            // highest number fd to use on select function on linux
            SOCKET _nfds;
    };
}

#endif
