#ifndef __PROTOCOL_METHOD_H__
#define __PROTOCOL_METHOD_H__
#include "protocol_comm.h"
#include "protocol_packet_def.h"

// thread will call !!!!!!
class ProtocolMethod: public CProtocolComm
{
public:
    enum
    {
        OFF = 0,
        ON = 1, 
        STOP = 2,
        NONE = 3,

        MODE_COLD = 0,
        MODE_HOT = 1,

        // 硬件类型
        HD_TYPE_A = 0x0A,
        HD_TYPE_B = 0x0B,
        HD_TYPE_C = 0x0C,
        HD_TYPE_D = 0x0D,
    };

public:
    ProtocolMethod();

    // @param
    //   nDestAddr  1 - 8  目标地址
    //   nIndex      0 - 6  灯序号（共6个灯），0表示所有灯
    //   mode        ON/OFF 开关灯
    bool light_switch(int nDestAddr, int nIndex, int mode);

    struct light_switch_status
    {
        // 灯状态
        int light1_status;
        int light2_status;
        int light3_status;
        int light4_status;
        int light5_status;
        int light6_status;
    };
    // @param
    //   nDestAddr  1 - 8  目标地址
    bool query_light_switch(int nDestAddr, light_switch_status &status, bool bSaveInBuffer=false);

    //窗帘
    // @param
    //   inner_mode   ON/OFF/STOP/NONE  内窗帘
    //   outside_mode ON/OFF/STOP/NONE  外窗帘
    //   bHasOne      有无人
    //   nSeason      1-冬季 0-夏季
    //   bSunshine    是否有阳光
    //   dOutsideTemp 室外温度(-25 - 40)
    bool curtain(int nDestAddr, int inner_mode, int outside_mode, 
                 bool bHasOne, int nSeason, bool bSunshine, double dOutsideTemp);

    // 窗帘状态查询
    struct curtain_status
    {
        // 0 - 全关
        // 1 - 全开
        // 2 - 半开
        // 3 - 正开
        // 4 - 正关
        int inner; // 内窗帘状态  
        int outside; // 外窗帘状态

        // 0 - 无故障
        // 1 - 有故障
        int inner_fault; // 内窗帘故障
        int outside_fault; // 外窗帘故障

        // 0 - 无人
        // 1 - 有人
        int has_man;

        // 0 - 手动控制
        // 1 - 自动控制
        int control;

        // 0 - 夏季
        // 1 - 冬季
        int season;

        // 0 - 无
        // 1 - 有
        int sunshine;

        // 室外温度
        double outside_temp;
    };
    bool query_curtain(int nDestAddr, curtain_status &status, bool bSaveInBuffer=false);

    // 抄表
    struct meter_status
    {
        double cold_cur_rate; // 冷水瞬时流量                    m3/h
        double hot_cur_rate; // 热水瞬时流量                     m3/h
        double cold_totoal;  // 冷水累计流量                     m3
        double hot_totoal;   // 热水累计流量                     m3
        double hot_input_temp; // 热水入口温度                   c
        double hot_output_temp; // 热水出口温度                  c
        double hot_totoal_degree; // (热水累计热量）热水的总度数 kWh
        double hot_cur_power; // (热水瞬时热量） 热水的当前功率  kW
        double cur_totoal_power; // 瞬时总有功功率               kW
        double forward_totoal_degree; // (正向有功总电能) 正向总度数 kWh
        double gas_cur_rate; // 燃气当前流量                     m3/h
        double gas_totoal_rate; // 燃气总流量                    m3
    };
    bool query_meter(int nDestAddr, meter_status &status, bool bSaveInBuffer=false);

    // 时间同步抄表
    bool time_syn_meter(int nDestAddr);

    // 空调控制
    // switch_mode        ON/OFF/NONE
    // aircondition_mode  MODE_HOT/MODE_COLD
    // xinfeng_mode       1-纯新风 2-纯空调 3-新风+空调
    // bHasOne              有人无人
    // energy_save_mode     节能模式
    // inner_kongguang_temp 空管器室内温度
    bool aircondition(int nDestAddr, int switch_mode, int aircondition_mode, int xinfeng_mode, int temp,
                      bool bHasOne, int energy_save_mode, int inner_kongguang_temp, bool bFireAlarm);

    // 查询空调状态
    struct aircondition_status
    {
        int switch_mode;
        int aircondition_mode;
        int xinfeng_mode;
        int temp_setting;

        int energy_save_mode; // 节能模式

        int nXinfengSwitch; // 新风閥状态
        int nTiaofengSwitch;// 调风
        int nPaifengSwitch; // 排风

        // 火警 
        bool bFireAlarm;
        // 新风阀故障 ... 调风阀通信故障
        BYTE btAlarm1;
        // 排风阀通信故障
        BYTE btAlarm2;
    };
    // bHasOne              有人无人
    // inner_kongguang_temp 空管器室内温度
    bool query_aircondition(int nDestAddr, aircondition_status &status, bool bHasOne, 
                            int inner_kongguang_temp, bool bSaveInBuffer=false);
    // 灯控器升级模式
    bool light_upgrade(int nDestAddr);

    // 查询灯控器类型
    bool light_type(int nDestAddr, int &type);
    
    // 帘控器升级模式
    bool curtain_upgrade(int nDestAddr);

    // 查询帘控器类型
    bool curtain_type(int nDestAddr, int &type);

    // 抄表接口升级模式
    bool meter_upgrade(int nDestAddr);

    // 发送485数据包，bSaveInBuffer如果是True就更新缓存
    bool transform(BYTE btSrcAddr, BYTE btDestAddr,
                   BYTE btType, BYTE btFunction, void *pData, BYTE btLen, Packet &packet_ret, bool bSaveInBuffer=false, bool bStopResendTemp=false);

    bool transform_by_kongguan(Packet &packet, Packet &packet_ret);

    void SetCmdPacketTransMap(BYTE from_type, BYTE from_function, BYTE to_type, BYTE to_function);

protected:
    struct HD_TYPE
    {
        int type;
    };
    bool query_type(int nDestAddr, int typeCode, HD_TYPE &type);

protected:
    // 返回一个字节应答
    //bool GetResponseNByte(BYTE btCmd, BYTE *pByte, int nLen);

    //超时等待应答并检查结果
    bool WaitCheckResult(int nDestAddr, BYTE btType, BYTE btFunction, Packet &packet);

    //转换BCD值
    unsigned int BCD(const BYTE *pByte, int nLen);

private:
    // lock to protect 485 SendFrame
    lib_linux::Mutex m_mutex;

    // lock to protect m_queryPacketMap m_queryPacketBlockSet
    lib_linux::Mutex m_query_mutex;

    //查询数据包缓存
    std::map<DWORD, Packet> m_queryPacketMap;

    //屏蔽这些转发, 只能查缓存 type<<8|function
    // DWORD = btType<<8|btFunction
    std::set<DWORD> m_queryPacketBlockSet;

    //转换命令返回数据包到查询数据缓存中
    // DWORD = btType<<8|btFunction
    std::map<DWORD, DWORD> m_cmdPacketTransMap;
};
#endif
