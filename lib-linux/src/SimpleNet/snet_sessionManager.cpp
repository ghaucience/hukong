#include "snet_config.h"
#include "snet_sessionManager.h"
#include "snet_tool.h"

namespace lib_linux
{
    SessionManager::SessionManager(int nMaxSession)
        :m_pFactoryServSession(NULL),
        _max_fd(nMaxSession),
        _nfds(INVALID_SOCKET)
    {
        // empty operation
        system::Initial();

        ASSERT(_max_fd >= 1);

        // initial FD_SET array, default size is 64
        // add listen fd
        _read_set.set_size(_max_fd+1);
        _write_set.set_size(_max_fd);
        _error_set.set_size(_max_fd);
    }

    SessionManager::~SessionManager()
    {
        Shutdown();
    }

    bool SessionManager::IsRunning()
    {
        return _listen.is_valid();
    }

    SessionManager::SessionList SessionManager::SessionMapToList(SessionMap &map)
    {
        SessionList list;
        for (SessionMap::iterator it=map.begin(); it!=map.end(); it++)
        {
            list.push_back(it->second);
        }
        return list;
    }

    int SessionManager::GetSessionCount()
    {
        return _sessions.size();
    }

    int SessionManager::GetWaitingNBSessionCount(void)
    {
        return _nbconnect_sessions.size();
    }

    SessionManager::SessionList SessionManager::GetSessionList()
    {
        return SessionMapToList(_sessions);
    }

    bool SessionManager::StartupServer(SessionFactory *pFactory, const char *pstrIP, unsigned short usPort)
    {
        // shutdown before start
        Shutdown();

        // server session factory
        m_pFactoryServSession = pFactory;

        // create the socket
        if( !_listen.create() )
        {
            return false;
        }

        Address addr(pstrIP, usPort);
        if (!addr.is_valid())
        {
            return false;
        }

        // bind address
        if (!_listen.bind(addr))
        {
            return false;
        }

        // set the reuse address option.
        int reuseaddr = 1;
        ::setsockopt(_listen.query_socket(), SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)); 

        // listen
        if (!_listen.listen(20))
        {
            return false;
        }

        return true;
    }

    ISession *SessionManager::Connect(SessionFactory &factory, const Address &remote_addr, bool bBlocking)
    {
#if defined(_WIN32)
        // check listen socket is valid or create it
        if (!_listen.is_valid())
        {
            if (!_listen.create())
            {
                return NULL;
            }
        }
#endif

        ISession *session = factory.CreateSession();
        if (session->create())
        {
            if ((int)(_sessions.size()+_nbconnect_sessions.size()) < _max_fd)
            {
                if (bBlocking)
                {
                    if (session->connect(remote_addr))
                    {
                        _sessions[session->getid()] = session;
                        session->OnConnect(this);
                        return session;
                    }
                }
                else
                {
                    if (session->connect_nb(remote_addr))
                    {
                        // 通过select的write和error fdset判断是否连接
                        _nbconnect_sessions[session->getid()] = session;
                        return session;
                    }
                }
            }
            else
            {
                ERROR("Connect numbers exceed max");
                session->OnError(this, WSAGetLastError(), "Connect numbers exceed max");
            }
        }

        delete session;
        return NULL;
    }

    void SessionManager::Shutdown()
    {
        //TRACE("_sessions: %d, _nbconnect_sessions: %d", _sessions.size(), _nbconnect_sessions.size());
        // close all session
        SessionList list = SessionMapToList(_sessions);
        for (SessionList::iterator it=list.begin(); it!=list.end(); it++)
        {
            // delete session
            ISession *session = *it;
            _sessions.erase(session->getid());
            session->OnDisconnect(this);
            delete session;
        }

        // clean up non-blocking connection session
        list = SessionMapToList(_nbconnect_sessions);
        for (SessionList::iterator it=list.begin(); it!=list.end(); it++)
        {
            // because session not connected, so don't call OnDisconnect
            delete *it;
        }
        _nbconnect_sessions.clear();

        // delete session
        _delete_sessions.clear();

        // close listen socket
        _listen.release();

        // clear session factory
        m_pFactoryServSession = NULL;

        _nfds = INVALID_SOCKET;
    }

    void SessionManager::SetupFdset()
    {
        _read_set.clear();
        _write_set.clear();
        _error_set.clear();

        if (_listen.is_valid())
        {
            _read_set.add( _listen.query_socket() );
            //_write_set.add( _listen.query_socket() );
            _nfds = _listen.query_socket();
        }

        for (SessionMap::iterator it=_sessions.begin(); it!=_sessions.end(); it++)
        {
            ISession *session = it->second;
            _read_set.add(session->query_socket());
            //_write_set.add(session->query_socket());

            if (_nfds < session->query_socket())
            {
                _nfds = session->query_socket();
            }
        }

        //ASSERT(_read_set.count() > 0 && "_read_set count is 0");

        // add non-blocking connection socket
        for (SessionMap::iterator it=_nbconnect_sessions.begin();
                it!=_nbconnect_sessions.end();
                it++)
        {
            ISession *session = it->second;
            _write_set.add(session->query_socket());
            _error_set.add(session->query_socket());

            if (_nfds < session->query_socket())
            {
                _nfds = session->query_socket();
            }
        }
    }

    int SessionManager::Poll(struct timeval &timeout)
    {
        SetupFdset();
        int ret = ::select( _nfds+1, _read_set, _write_set, _error_set, &timeout);

        if (ret > 0)
        {
            // new connection comes ?
            if (_listen.is_valid() &&  _read_set.is_set( _listen.query_socket() ) )
            {
                DEBUG("new session");
                NewSession();
            }

            SessionList list = SessionMapToList(_sessions);
            // poll read session
            for (SessionList::iterator it=list.begin(); it!=list.end(); it++)
            {
                ISession *session = *it;
                SOCKET s = session->query_socket();

                if (_read_set.is_set(s))
                {
                    // notify the listener
                    int nReturn = session->do_recv();
                    if (nReturn <= 0 )
                    {
                        DEBUG("session close connection");
                        if (nReturn == SOCKET_ERROR)
                        {
                            ERROR("socket wrong and close connection");
                            session->OnError(this, WSAGetLastError(), "socket wrong and close connection" );
                        }

                        // delete session
                        _sessions.erase(session->getid());
                        session->OnDisconnect(this);
                        delete session;
                    }
                    else
                    {
                        session->OnRead(this);
                    }
                }
            } //for

            // poll write and error session
            list = SessionMapToList(_nbconnect_sessions);
            for (SessionList::iterator it=list.begin(); it!=list.end(); it++)
            {
                ISession *session = *it;
                SOCKET s = session->query_socket();

                if (_error_set.is_set(s)) // error
                {
                    _nbconnect_sessions.erase(session->getid());
                    ERROR("connection non-blocking socket error");
                    session->OnError(this, WSAGetLastError(), "connection non-blocking socket error");
                    delete session;
                }
                else if (_write_set.is_set(s)) // write 
                {
                    // check error (when socket error it become writeable on linux)
                    // so must check.
                    int err = 0;
                    socklen_t errlen = sizeof(err);
                    ::getsockopt(s, SOL_SOCKET, SO_ERROR, &err, &errlen);
                    if (err)
                    {
                        _nbconnect_sessions.erase(session->getid());
                        ERROR("connection non-blocking socket error");
                        session->OnError(this, WSAGetLastError(), "connection non-blocking socket error");
                        delete session;
                    }
                    else
                    {
                        // delete from non-blocking connection session
                        _nbconnect_sessions.erase(session->getid());
                        _sessions[session->getid()] = session;
                        session->OnConnect(this);
                    }
                }
            } //for

            // delete session of _delete_sessions
            for (SessionMap::iterator it=_delete_sessions.begin(); it!=_delete_sessions.end(); it++)
            {
                int id = it->first;
                ISession *session = it->second;

                if (_sessions.count(id) > 0)
                {
                    // delete session
                    _sessions.erase(id);
                    session->OnDisconnect(this);
                    delete session;
                }
                else if (_nbconnect_sessions.count(id) > 0)
                {
                    // just delete session
                    _nbconnect_sessions.erase(id);
                    delete session;
                }
            } //for
            _delete_sessions.clear();
        } // if

        return ret;
    }

    void SessionManager::Delete(ISession *pSession)
    {
        assert(_delete_sessions.count(pSession->getid()) == 0);
        _delete_sessions[pSession->getid()] = pSession;
    }


    void SessionManager::DeleteDirect(ISession *pSession)
    {
        assert(pSession != NULL && _sessions.count(pSession->getid()) > 0);
        // delete session
        _sessions.erase(pSession->getid());
        pSession->OnDisconnect(this);
        delete pSession;
    }

    bool SessionManager::NewSession()
    {
        ASSERT(m_pFactoryServSession && "No server session factory method set");
        ISession *session = m_pFactoryServSession->CreateSession();
        if (session->accept(_listen))
        {
            if ((int)(_sessions.size()+_nbconnect_sessions.size()) < _max_fd)
            {
                // set tcp keep alive
                session->SetKeepAlive();
                _sessions[session->getid()] = session;
                // notify connect session
                session->OnConnect(this);
                return true;
            }
            else
            {
                ERROR("accept numbers excced max");
                session->OnError(this, WSAGetLastError(), "accept numbers excced max");
            }
        }

        delete session;
        return false;
    }

    void SessionManager::Broadcast(const char *buf, int len, const ISession *pExclude)
    {
        // only session in _session ready to broadcast 
        SessionList list = SessionMapToList(_sessions);
        for (SessionList::iterator it=list.begin(); it!=list.end(); it++)
        {
            if (pExclude != NULL && (*it) == pExclude)
            {
                continue;
            }

            (*it)->send( buf, len );
        }
    }
}
