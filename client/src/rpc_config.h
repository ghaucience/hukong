#ifndef __RPC_CONFIG_H__
#define __RPC_CONFIG_H__

#define RPC_MAJOR   1
#define RPC_MINOR   7
#define RPC_RELEASE 4

#define _STR(x)  #x
#define STR(x)  _STR(x)
#define RPC_VERSION  STR(RPC_MAJOR)"."STR(RPC_MINOR)"."STR(RPC_RELEASE)

#define RPC_METHOD_CALL_RETURN_FAIL  -32500

// cmd param
enum MODE
{
    ON = 0x01, //开
    OFF = 0x00, //关

    MODE_COLD = 0x00, //制冷
    MODE_HOT = 0x01, //制热

    FAULT = 0x01, //故障
    NO_FAULT = 0x00, //无故障
};

enum
{
    // 灯光
    TYPE_LIGHT = 0,
    // 窗帘
    TYPE_CURTAIN,
    // 带新风功能的空调
    TYPE_AIRCONDITION,
    // 抄表接口
    TYPE_METER,

    // 人感
    TYPE_HUMAN,
    // 漏水检查探头
    TYPE_LEAK_WATER,
    // 燃气检查探头
    TYPE_LEAK_GAS,
    TYPE_NULL,
};
#endif
