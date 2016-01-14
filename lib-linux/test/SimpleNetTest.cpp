#include <iostream>
#include <sstream>
#include <signal.h>
#include <unistd.h>
#include "lib_linux_config.h"
#include "snet.h"

using namespace std;
using namespace lib_linux;

class MySession:public ISession
{
    public:
    protected:
        void OnConnect(SessionManager *pSessionManager)
        {
            cout<<get_peer_addr().get_ip().c_str()<<"new session join in, online sessions:"
                <<pSessionManager->GetSessionCount()<<endl;

            //std::string str("Welcome to lbzhung's chat server:D\r\n");
            //send(str.c_str(), str.length());
        }

        void OnDisconnect(SessionManager *pSessionManager)
        {
            cout<<get_peer_addr().get_ip().c_str()<<"exit, online sessions:"
                <<pSessionManager->GetSessionCount()-1<<endl;
        }

        void OnRead(SessionManager *pSessionManager)
        {
            char buffer[256];
            string strCur;

            while (true)
            {
                int nLen = recv(buffer, sizeof(buffer)-1);
                if (nLen <= 0)
                {
                    break;
                }

                buffer[nLen] = '\0';
                strCur += buffer;
            }

            pSessionManager->Broadcast(strCur.c_str(), strCur.size(), this);
        }

        void OnError(SessionManager *pSessionManager, int nErrorCode, const char *pStr)
        {
            printf("error: %s\n", pStr);
        }
};

volatile static bool is_exit = false;
static void onsig(int dummy)
{
    is_exit = true;
}

void usage(const char *pstrProgram)
{
    printf("Usage:\n"
           "  %s -p port \n"
           "     -d daemon\n", pstrProgram);
}

int main(int argc, char* argv[])
{
    int opt = -1;
    int nPort = -1;
    bool isDaemon = false;
    while ((opt = getopt(argc, argv, "hdp:")) != -1)
    {
        switch (opt)
        {
            case 'd':
                isDaemon = true;
                break;
            case 'p':
                nPort = atoi(optarg);
                break;
            case 'h':
                usage(argv[0]);
                return 0;
            default: /* '?' */
                usage(argv[0]);
                return -1;
        }
    } 

    if (nPort == -1)
    {
        usage(argv[0]);
        return -1;
    }

    ::signal(SIGHUP, onsig);
    ::signal(SIGINT, onsig);
    ::signal(SIGTERM, onsig);

    // socket send will throw SIGPIPE if remote is close
    ::signal(SIGPIPE, SIG_IGN);

    if (isDaemon)
    {
        // daemonize
        if (!lib_linux::Utility::Daemonize())
        {
            return -1;
        }
    }

    SET_LOG(lib_linux::FLAG_CON | lib_linux::FLAG_COLOR, DEBUG);

    SessionFactoryImp<MySession> sm;
    SessionManager manager(10);

    // Start chat server and listen at port.
    // You can login by "telnet localhost port".
    if (!manager.StartupServer(&sm, "0.0.0.0", nPort))
    {
        return 1;
    }

    while (!is_exit)
    {
        timeval timeout = { 0, 0};
        manager.Poll(timeout);
        usleep(10*1000);

        for (int i=0; i<204; i++)
        {
            manager.Broadcast("hello", 5);
        }
    }

    return 0;
}
