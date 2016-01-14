#ifndef __RPC_CLIENT_H__
#define __RPC_CLIENT_H__

#include <stdbool.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>
#include "config.h"  /* information about this build environment */
#include "rpc_config.h"

struct rpc_client
{
    xmlrpc_client *m_pClient;
    xmlrpc_env m_env;
    char *m_pstrURL;
 
    // manage string result
    const char *m_pstrSave;
};
typedef struct  rpc_client rpc_client;

extern int rpc_client_init(rpc_client *pRPC);
extern void rpc_client_destroy(rpc_client *pRPC);
extern void rpc_client_set_url(rpc_client *pRPC, const char *pstrURL);
extern int rpc_is_call_ok(rpc_client *pRPC);
extern void rpc_print_error(rpc_client *pRPC);

//////////////////////////////////////////////////////////////////
// RPC call implement

// 得到客户端RPC版本
extern const char *protocol_version(rpc_client *pRPC);

// 检查客户端RPC版本是否兼容服务端
extern int protocol_version_is_ok(rpc_client *pRPC);


// 开关灯
// @param
//   nDestAddr  1 - 8  目标地址
//   nIndex      0 - 6  灯序号（共6个灯），0表示所有灯
//   mode        1/0 开/关
extern void light_switch(rpc_client *pRPC, int nDestAddr, int nIndex, int mode);

typedef struct _light_switch_status
{
    // 灯状态
    int light1_status;
    int light2_status;
    int light3_status;
    int light4_status;
    int light5_status;
    int light6_status;
}light_switch_status;
// 查询灯状态
// @param
//   nDestAddr  1 - 8  目标地址
extern light_switch_status query_light_switch(rpc_client *pRPC, int nDestAddr);

// 窗帘开关
// @param
//   nDestAddr  目标地址
//   index      0-内窗帘  1-外窗帘
//   mode       0/1/2     关/开/停
extern void curtain_switch(rpc_client *pRPC, int nDestAddr, int index, int mode);


typedef struct _curtain_status
{
    // 0-全关  1-全开  2-半开 3-正开  4-正关
    int inner; // 内窗帘状态  
    int outside; // 外窗帘状态

    // 0 - 无故障
    // 1 - 有故障
    int inner_fault; // 内窗帘故障
    int outside_fault; // 外窗帘故障
}curtain_status;
// 窗帘状态查询
// @param
//   nDestAddr  1 - 8  目标地址
extern curtain_status query_curtain(rpc_client *pRPC, int nDestAddr);

typedef struct _meter_status { 
    double cold_cur_rate; // 冷水瞬时流量                        0.0001m3/h
    double hot_cur_rate; // 热水瞬时流量                         0.0001m3/h
    double cold_totoal;  // 冷水累计流量                         0.01m3
    double hot_totoal;   // 热水累计流量                         0.01m3
    double hot_input_temp; // 热水入口温度                       0.01C
    double hot_output_temp; // 热水出口温度                      0.01C
    double hot_totoal_degree; // (热水累计热量）热水的总度数     0.01kWh
    double hot_cur_power; // (热水瞬时热量） 热水的当前功率      0.01kW
    double cur_totoal_power; // 瞬时总有功功率                   0.0001kW
    double forward_totoal_degree; // (正向有功总电能) 正向总度数 0.01kWh
    double gas_cur_rate; // 燃气当前流量                         0.0001m3/h
    double gas_totoal_rate; // 燃气总流量                        1m3
}meter_status;
// 抄表
// @param
//   nDestAddr  1 - 4  目标地址
extern meter_status query_meter(rpc_client *pRPC, int nDestAddr);

// 空调控制
// switch_mode         1/0 开/关
// aircondition_mode   0-制冷 1-制热
// xinfeng_mode        1-纯新风 2-纯空调 3-新风+空调
// temp                温度(5-35C)
// energy_save_mode;   空调节能模式 0--8小时关 1--1小时关 2--4小时关 3--24小时关 4-人离不关
extern void aircondition(rpc_client *pRPC, int nDestAddr, int switch_mode, int aircondition_mode, int xinfeng_mode, int temp, int energy_save_mode); 

// 查询空调状态
typedef struct _aircondition_status
{
    int switch_mode;       // 开关状态 1/0 开/关
    int aircondition_mode; // 0-制冷 1-制热
    int xinfeng_mode;      // 1-纯新风 2-纯空调 3-新风+空调
    int temp_setting;      // 温度(5-35C)
    int energy_save_mode;  // 空调节能模式 0--8小时关 1--1小时关 2--4小时关 3--24小时关 4-人离不关
}aircondition_status;
extern aircondition_status query_aircondition(rpc_client *pRPC, int nDestAddr);


//////////////////////////////////////////////////////////////////////////
typedef struct _DevInfo
{
    int type; // 设备类型
    int addr; // 485地址
    int opt; // 设备参数（如果是灯控器，存储灯的序号）
}DevInfo;
// 查找人感对应的设备
// @param
//   nSenID  人感ID
//   pInfo   用户传入数组指针
//   nLen    用户传入数组长度
// @return
//   返回绑定的条数
//   -1 传入nLen太小
extern int humansen_to_dev(rpc_client *pRPC, int nSenID, DevInfo *pInfo, int nLen);

// 人感绑定到设备是否改变，如果改变再去调humansen_to_dev进行更新
// @return
//   是否改变(bool)
extern int is_bind_change(rpc_client *pRPC);

// 返回人感状态
// @return
//    返回人感个数
//        人感状态数组pData, 1-有人 0-无人, 人感ID对应数值索引+1，比如数组0对应1号人感
//    -1 传入nLen太小
extern int get_sen_status(rpc_client *pRPC, int *pData, int nLen);


// 返回检测到无人后到关灯时间
// @return
//   int    时间，单位是秒
extern int get_light_off_time(rpc_client *pRPC);

// 获得设备的个数
typedef struct _DevCount
{
    int type; // 设备类型
    int count; // 设备个数
}DevCount;
// @return
//    返回设备结构体数组个数
//    -1 传入nLen太小
extern int get_device_count(rpc_client *pRPC, DevCount *pCount, int nLen);

// 空气品质
typedef struct _air_quality
{
    int inner_PM03;    // PM0.3  分辨率1ppm
    int inner_PM25;    // PM2.5  分辨率1ppm
    int inner_PM10;    // PM10   分辨率1ppm
    double inner_VOC;  // 甲醛VOC浓度 分辨率0.01ppm
    int inner_CO2;     // 二氧化碳浓度 分辨率1ppm
    int inner_temp;    // 室内温度
    int inner_humidity; // 室内湿度 1%
}air_quality;
extern air_quality get_air_quality(rpc_client *pRPC);

// 获得新风运行定时参数
typedef struct _xinfeng_time
{
    int nOFF; //人离阀关时间 单位秒
    int nON; //自动开阀时间 单位秒
}xinfeng_time;
extern xinfeng_time get_xinfeng_time(rpc_client *pRPC);

// 获得新风阀和调风阀状态
typedef struct _xinfeng_switch_status
{
    int nXinfengSwitch; // 新风阀状态
    int nTiaofengSwitch; // 调风阀状态
}xinfeng_switch_status;
extern xinfeng_switch_status get_xinfeng_switch_status(rpc_client *pRPC, int nDestAddr);

#endif
