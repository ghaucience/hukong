#include <stdexcept>
#include <cassert>
#include <cstdio>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include "utility.h" //lib_linux
#include "lib_linux_config.h"
#include "repeat_comm.h"
#include "rpc_config.h"

extern int rpc_method_init(xmlrpc_c::registry &, RepeatComm &);

void usage(const char *pstrProgram)
{
    printf("Hukong system Version %s. Copyright (C) 2013 Lierda Science& Technology Group Co., Ltd\n", VERSION);
    printf("Usage:\n"
           "  %s [opt] -t /dev/ttyXXX /dev/ttyXXX\n"
           "    -d           daemon mode\n"
           "    -p port      xmlrpc server port\n"
           "    -b baudrate  485 serialport baudrate\n"
           "    -t ttyXXX    kongguan 485 serialport\n"
           "    -l destaddr  light upgrade 485 addr\n"
           "    -c destaddr  curtain upgrade 485 addr\n"
           "    -m destaddr  meter upgrade 485 addr\n"
           "    -V           show verbose debug infomation\n"
           "    -S           scan all device and print infomation\n"
           "    -h           show this help\n"
           "    -v           print version\n"
           "\n"
           "Example:\n"
           "    %s -d -t /dev/ttyUSB2 /dev/ttyUSB0\n"
           "scan all devs\n"
           "    %s -S /dev/ttyUSB0\n"
           "get light dev hardware type\n"
           "    %s -S -l 1 /dev/ttyUSB0\n",
           pstrProgram,pstrProgram,pstrProgram,pstrProgram);
}

int main(int argc, char *argv[])
{
    int opt = -1;
    const char *pstrSerial = NULL;
    const char *pstrTranSerial = NULL;
    int nBaudrate = 9600;
    int nPort = 8000;
    bool isDaemon = false;
    int nDevUpgradeType = RepeatComm::TYPE_NULL;
    int nDevUpgradeAddr = 0;
    bool bDevUpgrade = false;
    bool bVerbose = false;
    bool bScanDev = false;
    while ((opt = getopt(argc, argv, "dt:p:b:sSl:c:m:hvV")) != -1)
    {
        switch (opt)
        {
            case 'd':
                isDaemon = true;
                break;
            case 'p':
                nPort = atoi(optarg);
                break;
            case 'b':
                nBaudrate = atoi(optarg);
                break;
            case 't':
                pstrTranSerial = optarg;
                break;
            case 's':
                // 废弃标志兼容以前版本
                break;
            case 'S':
                bScanDev = true;
                break;
            case 'l':
            case 'c':
            case 'm':
                if (bDevUpgrade)
                {
                    usage(argv[0]);
                    return -1;
                }
                bDevUpgrade = true;
                if (opt == 'l')
                {
                    nDevUpgradeType = RepeatComm::TYPE_LIGHT;
                }
                else if (opt == 'c')
                {
                    nDevUpgradeType = RepeatComm::TYPE_CURTAIN;
                }
                else if (opt == 'm')
                {
                    nDevUpgradeType = RepeatComm::TYPE_METER;
                }
                nDevUpgradeAddr = atoi(optarg);
                break;
            case 'h':
                usage(argv[0]);
                return 0;
            case 'V':
                bVerbose = true;
                break;
            case 'v':
                printf("version: %s\n", VERSION);
                printf("last modify: %s\n", VER_DATE);
                return 0;
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

    // scan device result must show on console
    if (isDaemon && !bScanDev)
    {
        // daemonize
        if (!lib_linux::Utility::Daemonize())
        {
            ERROR("daemonize fail\n");
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
        DEBUG("daemonize");
    }
    else
    {
        if (bScanDev)
        {
            // don't output log
            SET_LOG(lib_linux::FLAG_CON, NONE);
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
    }

    try {
        RepeatComm repeatComm;
        if (!repeatComm.OpenDevice485(pstrSerial, nBaudrate))
        {
            ERROR("open device 485 fail");
            return -1;
        }

        if (bScanDev)
        {
            if (pstrTranSerial != NULL)
            {
                return -1;
            }

            if (nDevUpgradeAddr > 0 &&
                (nDevUpgradeType == RepeatComm::TYPE_LIGHT ||
                nDevUpgradeType == RepeatComm::TYPE_CURTAIN)
               )
            {
                int type;
                bool ret = false;
                // get hardware type
                if (nDevUpgradeType  == RepeatComm::TYPE_LIGHT)
                {
                    ret = repeatComm.GetMethod().light_type(nDevUpgradeAddr, type);
                }
                else if (nDevUpgradeType == RepeatComm::TYPE_CURTAIN)
                {
                    ret = repeatComm.GetMethod().curtain_type(nDevUpgradeAddr, type);
                }

                if (ret)
                {
                    switch (type)
                    {
                        case ProtocolMethod::HD_TYPE_A:
                            printf("A\n");
                            break;
                        case ProtocolMethod::HD_TYPE_B:
                            printf("B\n");
                            break;
                        case ProtocolMethod::HD_TYPE_C:
                            printf("C\n");
                            break;
                        case ProtocolMethod::HD_TYPE_D:
                            printf("D\n");
                            break;
                        default:
                            printf("unkown\n");
                            break;
                    }
                    return 0;
                }
                else
                {
                    return -1;
                }
            }
        }

        if (bDevUpgrade)
        {
            if (pstrTranSerial != NULL)
            {
                ERROR("transport serialport not be open in upgrade mode");
                return -1;
            }

            if (nDevUpgradeAddr > 0 &&
                (nDevUpgradeType == RepeatComm::TYPE_LIGHT ||
                nDevUpgradeType == RepeatComm::TYPE_CURTAIN ||
                nDevUpgradeType == RepeatComm::TYPE_METER)
                )
            {
                bool bRet = false;
                switch (nDevUpgradeType)
                {
                    case RepeatComm::TYPE_LIGHT:
                        bRet = repeatComm.GetMethod().light_upgrade(nDevUpgradeAddr);
                        break;
                    case RepeatComm::TYPE_CURTAIN:
                        bRet = repeatComm.GetMethod().curtain_upgrade(nDevUpgradeAddr);
                        break;
                    case RepeatComm::TYPE_METER:
                        bRet = repeatComm.GetMethod().meter_upgrade(nDevUpgradeAddr);
                        break;
                }

                if (bRet)
                {
                    INFO("send upgrade command success");
                    return 0;
                }
                else
                {
                    INFO("fail!");
                    return -1;
                }
            }

            usage(argv[0]);
            return -1;
        }

        // scan all dev
        int nLight = 0;
        int nCurtain = 0;
        int nMeter = 0;
        int nAircondition = 0;
        try
        {
            // scan all devices
            ProtocolMethod::light_switch_status status_l;
            printf("light: ");
            for (int addr=1; addr <= RepeatComm::COUNT_MAX_LIGHT; addr++)
            {
                if (repeatComm.GetMethod().query_light_switch(addr, status_l))
                {
                    printf("%d ", addr);
                    nLight++;
                }
            }
            printf("\n");

            ProtocolMethod::curtain_status status_c;
            printf("curtain: ");
            for (int addr=1; addr <= RepeatComm::COUNT_MAX_CURTAIN; addr++)
            {
                if (repeatComm.GetMethod().query_curtain(addr, status_c))
                {
                    printf("%d ", addr);
                    nCurtain++;
                }
            }
            printf("\n");

            ProtocolMethod::meter_status status_m;
            printf("meter: ");
            for (int addr=1; addr <= RepeatComm::COUNT_MAX_METER; addr++)
            {
                if (repeatComm.GetMethod().query_meter(addr, status_m))
                {
                    printf("%d ", addr);
                    nMeter++;
                }
            }
            printf("\n");

            ProtocolMethod::aircondition_status status_a;
            printf("aircondition: ");
            for (int addr=1; addr <= RepeatComm::COUNT_MAX_AIRCONDITION; addr++)
            {
                if (repeatComm.GetMethod().query_aircondition(addr, status_a, false, 25))
                {
                    printf("%d ", addr);
                    nAircondition++;
                }
            }
            printf("\n");
        }
        catch (const char *pstrErr)
        {
            printf("query param error\n");
        }
        // 如果只是单纯扫描设备，就退出
        if (bScanDev)
        {
            return 0;
        }

        // 设置空管器真实的设备个数
        repeatComm.SetDeviceCount(nLight, nCurtain, nMeter, nAircondition, false);

        // 打开空管器485转发
        if (pstrTranSerial != NULL)
        {
            if (!repeatComm.OpenKongguanqi485(pstrTranSerial, nBaudrate))
            {
                ERROR("open Kongguanqi 485 fail");
                return -1;
            }
        }

        // 启动定时查询线程
        repeatComm.StartQueryThread();

        xmlrpc_c::registry myRegistry;
        DEBUG("rpc method init...");
        rpc_method_init(myRegistry, repeatComm);
        DEBUG("rpc server start...");

        xmlrpc_c::serverAbyss myAbyssServer(xmlrpc_c::serverAbyss::constrOpt()
                .registryP(&myRegistry)
                .portNumber(nPort)
                .keepaliveTimeout(15)
                .keepaliveMaxConn(30));


        xmlrpc_c::serverAbyss::shutdown _shutdown(&myAbyssServer);
        // enable shutdown xmlrpc server
        myRegistry.setShutdown(&_shutdown);

        DEBUG("rpc server running");
        myAbyssServer.run();

        // call system.shutdown exit
        //assert(false);
    } catch (std::exception const& e) 
    {
        ERROR("xmlrpc httpserver Something failed.  %s", e.what());
    }

    return 0;
}
