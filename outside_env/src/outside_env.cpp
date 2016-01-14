#include <cstdlib>
#include "config.h"
#include "utility.h" //lib_linux
#include "protocol_comm.h"
#include "SimpleNet/snet.h"

class MySession:public lib_linux::ISession
{
    protected:
        void OnConnect(lib_linux::SessionManager *pSessionManager)
        {
            //DEBUG("new connect to %s", get_peer_addr().get_ip().c_str());
        }

        void OnDisconnect(lib_linux::SessionManager *pSessionManager)
        {
            //DEBUG("disconnect %s", get_peer_addr().get_ip().c_str());
        }

        void OnRead(lib_linux::SessionManager *pSessionManager)
        {
        }

        void OnError(lib_linux::SessionManager *pSessionManager, int nErrorCode, const char *pStr)
        {
            ERROR("server ErrorCode:%d  %s", nErrorCode, pStr);
        }
};


void usage(const char *pstrProgram)
{
    printf("outside env Version %s. Copyright (C) 2013 Lierda Science& Technology Group Co., Ltd\n", VERSION);
    printf("Usage:\n"
            "  %s [opt] /dev/ttyXXX\n"
            "    -d           daemon mode\n"
            "    -c           remote ip\n"
            "    -p           remote tcp port\n"
            "    -h           show this help\n"
            "    -v           print version\n"
            "    -V           show verbose debug infomation\n"
            "\n",
            pstrProgram);
}

int main(int argc, char *argv[])
{
    int opt = -1;
    bool isDaemon = false;
    bool bVerbose = false;
    //int nPort = 8001;
    const char *pstrSerial = NULL;
    const char *pstrRemoteIP = NULL;
    int nRemotePort = 0;
    while ((opt = getopt(argc, argv, "dc:p:hvV")) != -1)
    {
        switch (opt)
        {
            case 'd':
                isDaemon = true;
                break;
            case 'c':
                pstrRemoteIP = optarg;
                break;
            case 'p':
                nRemotePort = ::atoi(optarg);
                break;
            case 'h':
                usage(argv[0]);
                return 0;
            case 'v':
                printf("version: %s\n", VERSION);
                return 0;
            case 'V':
                bVerbose = true;
                break;
            default: /* '?' */
                usage(argv[0]);
                return -1;
        }
    } 
    if (optind < argc)
    {
        pstrSerial = argv[optind];
    }
    else
    {
        usage(argv[0]);
        return -1;
    }

    if (isDaemon)
    {
        fprintf(stdout, "daemonize\n");
        // daemonize
        if (!lib_linux::Utility::Daemonize())
        {
            fprintf(stderr, "daemonize fail\n");
            return -1;
        }

        if (bVerbose)
        {
            SET_LOG(lib_linux::FLAG_SYSLOG, DEBUG);
        }
        else
        {
            SET_LOG(lib_linux::FLAG_SYSLOG, INFO);
        }
    }
    else
    {
        if (bVerbose)
        {
            SET_LOG(lib_linux::FLAG_CON | lib_linux::FLAG_COLOR, DEBUG);
        }
        else
        {
            SET_LOG(lib_linux::FLAG_CON | lib_linux::FLAG_COLOR, INFO);
        }
    }

    CProtocolComm comm;
    if (!comm.Open(pstrSerial, 9600))
    {
        ERROR("Open serialport fail: %s", pstrSerial);
        return -1;
    }

    // 10分钟上传一次
    //int nIntervalMSTime = 10*60*1000;
    int nIntervalMSTime = 1*60*1000;
    //int nIntervalMSTime = 2 * 1000;
    int nMSTimeLeft = 0;
    lib_linux::SessionManager manager(10);
    lib_linux::SessionFactoryImp<MySession> sm;
    while (true)
    {
        if (nMSTimeLeft <= 0)
        {
            // 采集室外环境参数
            CProtocolComm::env_info info;
            if (comm.QueryEnv(info))
            {
                if (pstrRemoteIP != NULL && nRemotePort != 0)
                {
                    lib_linux::Address remote_addr = lib_linux::Socket::get_host_name(pstrRemoteIP);
                    remote_addr.set_port(nRemotePort);
                    if (remote_addr.is_valid())
                    {
                        DEBUG("ip %s port %d", remote_addr.get_ip().c_str(), remote_addr.get_port());
                        // 非阻塞连接服务器
                        lib_linux::ISession *pSession = manager.Connect(sm, remote_addr, true);
                        if (pSession)
                        {
                            char buffer[512];
                            ::memset(buffer, 0, sizeof(buffer));
                            int nLen = snprintf(buffer, sizeof(buffer), "{\"type\":\"99\",\"PM03\":\"%u\",\"PM25\":\"%u\",\"PM10\":\"%u\","
                                    "\"jiaquang\":\"%.2f\","
                                    "\"CO2\":\"%u\","
                                    "\"temp\":\"%.1f\","
                                    "\"wet\":\"%u\","
                                    "\"season\":\"%d\","
                                    "\"bEastsun\":\"%d\","
                                    "\"bSouthsun\":\"%d\","
                                    "\"bWetsun\":\"%d\","
                                    "\"bSunsen1Fault\":\"%d\","
                                    "\"bSunsen2Fault\":\"%d\","
                                    "\"bSunsen3Fault\":\"%d\","
                                    "\"bDustsenFault\":\"%d\","
                                    "\"bWetsenFault\":\"%d\","
                                    "\"bTempsenFault\":\"%d\"}",
                                    info.PM03,
                                    info.PM25,
                                    info.PM10,
                                    info.jiaquang,
                                    info.CO2,
                                    info.temp, 
                                    info.wet,
                                    info.season,
                                    info.bEastsun,
                                    info.bSouthsun,
                                    info.bWetsun,
                                    info.bSunsen1Fault,
                                    info.bSunsen2Fault,
                                    info.bSunsen3Fault,
                                    info.bDustsenFault,
                                    info.bWetsenFault,
                                    info.bTempsenFault);
                            if (nLen > 0 && nLen < sizeof(buffer))
                            {
                                DEBUG("query env upload str len:%d : %s", nLen,  buffer);
                                pSession->send(buffer, nLen);
                            }
                            manager.DeleteDirect(pSession);
                        }
                        else
                        {
                            ERROR("Connect建立阻塞Session失败");
                        }
                    }
                }
            }
            else
            {
                ERROR("QueryEnv fail");
            }

            nMSTimeLeft = nIntervalMSTime;
        }

        lib_linux::Utility::Sleep(100);
        nMSTimeLeft -= 100;
    }

    return 0;
}
