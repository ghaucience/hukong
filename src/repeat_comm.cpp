#include <asm/byteorder.h>
#include <cmath>
#include <time.h>
#include "repeat_comm.h"
#include "rpc_config.h"

RepeatComm::TimerQuery::TimerQuery(RepeatComm &comm)
:m_repeatcomm(comm),
m_bRunFlag(true)
{
#ifdef _TEST
    m_led.Open(LED2_GPIO);
#endif
}

RepeatComm::TimerQuery::~TimerQuery()
{
    m_bRunFlag = false;
    Wait();
}

void RepeatComm::TimerQuery::Run()
{
    std::deque<DevInfo> devInfos;
    std::map<int, bool> xinfengAlarmMap;
    while (m_bRunFlag)
    {
        if (m_repeatcomm.IsDevChange())
        {
            INFO("Update Dev counts in query thread");
            // 跟新设备数目
            DevCount devs[10];
            int nSpecCount = m_repeatcomm.GetDeviceCount(devs, sizeof(devs)/sizeof(devs[0]));
            devInfos.clear();
            for (int i=0; i<nSpecCount; i++)
            {
                switch(devs[i].type)
                {
                    case TYPE_LIGHT:
                    case TYPE_CURTAIN:
                    case TYPE_AIRCONDITION:
                    case TYPE_METER:
                        for (int addr=1; addr<=devs[i].count; addr++)
                        {
                            DevInfo info = {devs[i].type, addr, 0};
                            devInfos.push_back(info);
                        }
                        break;
                }
            }
        }

        if (m_repeatcomm.IsMeterSyncTime())
        {
            // 设置抄表时间同步标志
            for (std::deque<DevInfo>::iterator it=devInfos.begin(); 
                                               it!=devInfos.end();
                                               it++)
            {
                if ((*it).type == TYPE_METER)
                {
                    (*it).opt = 1;
                }
            }
        }

        if (devInfos.size() > 0)
        {
            // 定时查询
            DevInfo info = devInfos.front();
            try
            {
                int type = info.type;
                BYTE addr = info.addr;
                switch(type)
                {
                    case TYPE_LIGHT:
                        {
                            // 重发3次，遇到优先级高的空管器命令，停止发送
                            for (int i=0; i<3; i++)
                            {
                                ProtocolMethod::light_switch_status status;
                                if (m_repeatcomm.m_isKongguanSetCmd || 
                                    m_repeatcomm.GetMethod().query_light_switch(addr, status, true))
                                {
                                    break;
                                }
                            }
                        }
                        break;
                    case TYPE_CURTAIN:
                        {
                            // 重发3次，遇到优先级高的空管器命令，停止发送
                            for (int i=0; i<3; i++)
                            {
                                ProtocolMethod::curtain_status status;
                                if (m_repeatcomm.m_isKongguanSetCmd || 
                                    m_repeatcomm.GetMethod().query_curtain(addr, status, true))
                                {
                                    break;
                                }
                            }
                        }
                        break;
                    case TYPE_AIRCONDITION:
                        {
                            // 重发3次，遇到优先级高的空管器命令，停止发送
                            for (int i=0; i<3; i++)
                            {
                                if (m_repeatcomm.m_isKongguanSetCmd)
                                {
                                    break;
                                }

                                ProtocolMethod::aircondition_status status;
                                if (m_repeatcomm.query_aircondition(addr, status, true))
                                {
#ifdef _TEST
                                    if (addr == 1)
                                    {
                                        m_led.Turn(status.switch_mode);
                                    }
#endif

                                    // 火警报警, 如果新风的火警不对就更新状态
                                    //if (status.bFireAlarm != (m_repeatcomm.m_bFireAlarm || m_repeatcomm.m_bBroadcastFireAlarm))
                                    if (xinfengAlarmMap.count(addr) == 0)
                                    {
                                        xinfengAlarmMap[addr] = false;
                                    }

                                    bool bFireAlarm = m_repeatcomm.m_bFireAlarm || m_repeatcomm.m_bBroadcastFireAlarm;
                                    if (xinfengAlarmMap[addr] != bFireAlarm)
                                    {
                                        xinfengAlarmMap[addr] = bFireAlarm;
                                        DEBUG("firealarm %d", bFireAlarm);
                                        // 上传火警信号给新风 发三遍
                                        if (m_repeatcomm.aircondition(addr, ProtocolMethod::NONE, status.aircondition_mode, status.xinfeng_mode,
                                                    status.temp_setting, status.energy_save_mode))
                                            if (m_repeatcomm.aircondition(addr, ProtocolMethod::NONE, status.aircondition_mode, status.xinfeng_mode,
                                                        status.temp_setting, status.energy_save_mode)) 
                                                m_repeatcomm.aircondition(addr, ProtocolMethod::NONE, status.aircondition_mode, status.xinfeng_mode,
                                                        status.temp_setting, status.energy_save_mode);
                                    }

                                    break;
                                }
                            }
                        }
                        break;
                    case TYPE_METER:
                        {
                            if (info.opt)
                            {
                                info.opt = 0;
                                INFO("meter sync time addr:%d", addr);
                                m_repeatcomm.GetMethod().time_syn_meter(addr);
                            }
                            else
                            {
                                // 重发3次，遇到优先级高的空管器命令，停止发送
                                for (int i=0; i<3; i++)
                                {
                                    ProtocolMethod::meter_status status;
                                    if (m_repeatcomm.m_isKongguanSetCmd ||
                                        m_repeatcomm.GetMethod().query_meter(addr, status, true))
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                        break;
                    default:
                        {
                            //for (std::deque<DevInfo>::iterator it=devInfos.begin();
                            //                                   it!=devInfos.end();
                            //                                   it++)
                            //{
                            //    DEBUG("type=%d addr=%d", (*it).type, (*it).addr);
                            //}
                            ERROR("error type = %d", type);
                        }
                        //assert(0 && "unkown device type on Run()");
                }
            }
            catch (const char *pstrErr)
            {
                ERROR("in timer query fail:%s", pstrErr);
            }


            // 空管器设置命令，优先发送，延时防止占总线
            if (m_repeatcomm.m_isKongguanSetCmd)
            {
                WARNING("catch set cmd and bus sleep");
                m_repeatcomm.m_isKongguanSetCmd = false;
                lib_linux::Utility::Sleep(500);
            }

            devInfos.pop_front();
            devInfos.push_back(info);
            lib_linux::Utility::Sleep(300);
        }
        else
        {
            lib_linux::Utility::Sleep(1000);
        }
    }
}

RepeatComm::RepeatComm()
:m_bRunFlag(false),
m_bInnerAirDetection(false),
m_bFireAlarm(false),
m_bBroadcastFireAlarm(false),
m_isBindChange(true),
m_isDevChange(true),
m_isMeterSyncTime(false),
m_sync_xinfeng_temp(-1),
m_sync_xinfeng_mode(-1),
m_timerQuery(*this),
m_isKongguanSetCmd(false)
{
    memset(m_data_setting, 0, sizeof(m_data_setting));

    // 初始化工况查询的默认值
    // 粉尘间隔时间,每天两次
    m_data_setting[7] = SET_BITS(m_data_setting[7], 5, 7, 0x01);
    // 隔热帘4小时关
    m_data_setting[8] = SET_BITS(m_data_setting[8], 0, 2, 0x02);
    // 照明10min关
    m_data_setting[8] = SET_BITS(m_data_setting[8], 3, 5, 0x03);

    // 默认制冷
    m_data_setting[9] = SET_BIT(m_data_setting[9], 2, 1);

    // 默认开启人感探头控制和室外空气检测
    m_data_setting[10] = SET_BIT(m_data_setting[10], 6, 1);
    m_data_setting[10] = SET_BIT(m_data_setting[10], 7, 1);

    // 室内需求温度
    m_data_setting[11] = 25*2;
    // 风管截面积
    m_data_setting[12] = 18;
    // 新风空调运行参数（人离阀关时间）
    m_data_setting[14] = 8;
    // 新风空调运行参数（自动开阀时间）
    m_data_setting[15] = 30;
    // 空管器室内温度（温探）
    m_data_setting[27] = 25*2+50;
    // 甲醛报警浓度0.2ppm
    m_data_setting[30] = 20;
    // CO2报警浓度1800ppm
    *(WORD *)(&m_data_setting[32]) = __cpu_to_le16(1800);

    memset(m_data_query_setting, 0, sizeof(m_data_query_setting));

    memset(m_data_hukong, 0, sizeof(m_data_hukong));
    // 初始化户控
    memset(&(m_data_hukong[7]),  0b11101110, 18);
    memset(&(m_data_hukong[25]), 0b10011001, 28);
    memset(&(m_data_hukong[53]), 0b10011001, 4);

    // 默认窗帘朝东
    m_data_hukong[57] = 0xFF;
    m_data_hukong[58] = 0xFF;

    // 0个灯和窗帘
    m_data_hukong[59] = 0;
    m_data_hukong[60] = 0;
    // 1个新风
    m_data_hukong[61] = 1;
    // 1个抄表
    m_data_hukong[62] = 1;
    // 5个人感
    m_data_hukong[63] = 5;
    // 4个漏水检测探头
    m_data_hukong[64] = 4;
    // 楼层1，房号1
    m_data_hukong[65] = 1;
    m_data_hukong[66] = 1;

    // 默认关联1号人感
    m_data_hukong[7] = SET_BITS(m_data_hukong[7], 0, 3, 0x1);
    m_data_hukong[25] = SET_BITS(m_data_hukong[25], 0, 3, 0x1);
    m_data_hukong[53] = SET_BITS(m_data_hukong[53], 0, 3, 0x1);

    memset(m_data_xinfeng, 0, sizeof(m_data_xinfeng));

    // 默认不触发时的开关量
    BYTE btStatus = TranSwitchStatus(false);
    memset(m_data_switch_status, (btStatus==1)?0x0F:0x00, sizeof(m_data_switch_status));
    // 默认不触发
    memset(m_data_sen_status, 0x00, sizeof(m_data_sen_status));

    // 默认单价
    memset(m_data_calc_aircondition, 0x00, sizeof(m_data_calc_aircondition));
    memset(m_data_calc_power_gas, 0x00, sizeof(m_data_calc_power_gas));
    memset(m_data_calc_water, 0x00, sizeof(m_data_calc_water));
    // 空调计量
    *((WORD *)&m_data_calc_aircondition[28]) = __cpu_to_le16(0.1*100);
    *((WORD *)&m_data_calc_aircondition[30]) = __cpu_to_le16(0.5*100);
    // 电力天然气查询
    *((WORD *)&m_data_calc_power_gas[28]) = __cpu_to_le16(1.0*100);
    *((WORD *)&m_data_calc_power_gas[30]) = __cpu_to_le16(4.0*100);
    // 卫生热水、自来水查询
    *((WORD *)&m_data_calc_water[28]) = __cpu_to_le16(0.5*100);
    *((WORD *)&m_data_calc_water[30]) = __cpu_to_le16(5.0*100);

    ::memset(m_data_outside_env, 0xFF, sizeof(m_data_outside_env));
    ::memset(m_data_outside_sen, 0, sizeof(m_data_outside_sen));
    // 室外温度默认25度
    m_outside_temp = 25;
    m_btSeason = 0;
    // 东面阳光信号
    m_bEastSun = false;
    // 南面阳光信号
    m_bSouthSun = false;
    // 西面阳光信号
    m_bWestSun = false;

    // 初始化其他要保存的数据
    ::memset(&m_data_others, 0, sizeof(m_data_others));

    Load();

    // 防止默认窗帘朝向为空，朝东
    if (m_data_hukong[57] == 0)
    {
        m_data_hukong[57] = 0xFF;
    }

    if (m_data_hukong[58] == 0)
    {
        m_data_hukong[58] = 0xFF;
    }

    // DEBUG ONLY
    // 2号人感绑定1.2号灯
    //m_data_hukong[25] = SET_BITS(m_data_hukong[25], 4, 7, 0x2);
    // 2号人感绑定1.3号灯
    //m_data_hukong[26] = SET_BITS(m_data_hukong[26], 0, 3, 0x2);
    // 3号人感绑定1.4号灯
    //m_data_hukong[26] = SET_BITS(m_data_hukong[26], 4, 7, 0x3);
    // 2号人感接户控器开关输入2
    //m_data_hukong[7] = SET_BITS(m_data_hukong[7], 4, 7, 0x02);
    // 3号人感接灯控开关输入2
    //m_data_hukong[9] = SET_BITS(m_data_hukong[9], 4, 7, 0x03);
    // 1号人感关联1号窗帘
    //m_data_hukong[49] = SET_BITS(m_data_hukong[49], 0, 3, 0x01);
    // 1号人感关联2号窗帘
    //m_data_hukong[49] = SET_BITS(m_data_hukong[49], 4, 7, 0x01);
    // 1号人感关联3号窗帘
    //m_data_hukong[50] = SET_BITS(m_data_hukong[50], 0, 3, 0x01);
    // 2号人感关联4号窗帘
    //m_data_hukong[50] = SET_BITS(m_data_hukong[50], 4, 7, 0x02);
    // 1号人感接户控开关输入2,3
    //m_data_hukong[7] = SET_BITS(m_data_hukong[7], 4, 7, 0x01);
    //m_data_hukong[8] = SET_BITS(m_data_hukong[8], 0, 3, 0x01);
    // 3个新风
    //m_data_hukong[61] = 3;
    // 2号人感关联1号新风
    //m_data_hukong[53] = SET_BITS(m_data_hukong[53], 4, 7, 0x01);
    // 3号人感关联2号新风
    //m_data_hukong[54] = SET_BITS(m_data_hukong[54], 0, 3, 0x02);

    // 测试户控器漏水警报
    //m_data_hukong[7] = SET_BITS(m_data_hukong[7], 0, 3, 0b1001);
    //m_data_hukong[7] = SET_BITS(m_data_hukong[7], 4, 7, 0b1010);
    //m_data_hukong[8] = SET_BITS(m_data_hukong[8], 0, 3, 0b1011);
    //m_data_hukong[8] = SET_BITS(m_data_hukong[8], 4, 7, 0b1100);

    //DEBUG_HEX((char *)m_data_setting, 34);
    //DEBUG_HEX((char *)m_data_hukong, 68);
    
    //设置命令数据包转换查询缓存数据包 类型和功能号
    m_method.SetCmdPacketTransMap(0x1E, 0x81, 0x1E, 0x01);
    m_method.SetCmdPacketTransMap(0x19, 0x81, 0x19, 0x01);
    m_method.SetCmdPacketTransMap(0x1C, 0x81, 0x1C, 0x01);
}

RepeatComm::~RepeatComm()
{
    m_bRunFlag = false;
    Wait();
}

ProtocolMethod &RepeatComm::GetMethod()
{
    assert(m_method.IsOpen());
    return m_method;
}

bool RepeatComm::OpenDevice485(const char *pstrSerial, int nBaudrate)
{
    if (m_method.Open(pstrSerial, nBaudrate))
    {
        // enable device 485 resend
        m_method.EnableResend(true);
        return true;
    }
    else
    {
        ERROR("open 485 serialport fail");
    }

    return false;
}

bool RepeatComm::OpenKongguanqi485(const char *pstrSerial, int nBaudrate)
{
    if (pstrSerial != NULL)
    {
        if (m_comm.Open(pstrSerial, nBaudrate))
        {
            // start kongguan repeat
            m_bRunFlag = true;
            Start();
            return true;
        }
        else
        {
            ERROR("open 485 kongguang serialport fail");
        }
    }

    return false;
}

bool RepeatComm::StartQueryThread()
{
    // 开启定时查询线程
    m_timerQuery.Start();
    return true;
}

void RepeatComm::Save(int type, BYTE *pData, int nLen)
{
    int fd = open(DATA_FILE, O_WRONLY | O_CREAT);
    if (fd == -1)
    {
        ERROR("save data_file:%s fail", DATA_FILE);
        return;
    }

    INFO("Save config %d", type);

    lseek(fd, type * sizeof(DATA_RECORD), SEEK_SET);
    write(fd, pData, nLen);
    close(fd);
}

void RepeatComm::Load()
{
    unsigned int index = 0;
    DATA_RECORD temp;
    int fd = open(DATA_FILE, O_RDONLY);
    if (fd == -1)
    {
        INFO("load data_file:%s fail", DATA_FILE);
        return;
    }

    lseek(fd, DATA_SETTING * sizeof(DATA_RECORD), SEEK_SET);
    read(fd, temp, sizeof(DATA_RECORD));
    for (index=0; index<sizeof(DATA_RECORD); index++)
    {
        if (temp[index] > 0)
        {
            break;
        }
    }
    if (index < sizeof(DATA_RECORD))
    {
        memcpy(m_data_setting, temp, sizeof(DATA_RECORD));
    }

    lseek(fd, DATA_HUKONG * sizeof(DATA_RECORD), SEEK_SET);
    read(fd, temp, sizeof(DATA_RECORD));
    for (index=0; index<sizeof(DATA_RECORD); index++)
    {
        if (temp[index] > 0)
        {
            break;
        }
    }
    if (index < sizeof(DATA_RECORD))
    {
        memcpy(m_data_hukong, temp, sizeof(DATA_RECORD));
    }

    lseek(fd, DATA_OTHERS * sizeof(DATA_RECORD), SEEK_SET);

    DATA_OTHERS_STRUCT others_temp;
    ::memset(&others_temp, 0 ,sizeof(others_temp));
    if (read(fd, (BYTE *)(&others_temp), sizeof(others_temp)) == sizeof(others_temp))
    {
        INFO("read data others");
        m_data_others = others_temp;
    }

    close(fd);
}

BYTE RepeatComm::GetSwitchStatus()
{
    BYTE btValue = 0;
    int id_to_gpio[] = {12, 22, 23, 26};
    int size = (int)(sizeof(id_to_gpio)/sizeof(int));
    for (int i=0; i<size; i++)
    {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", id_to_gpio[i]);
        int fd = open(buffer, O_RDONLY);
        if (fd == -1)
        {
            ERROR("open gpio %d fail", id_to_gpio[i]);
            assert(false);
            return 0;
        }

        char value = 0;
        read(fd, &value, 1);
        // 转换gpio到开关量
        if (TranGPIOToSwitchStatus((value == '0')?0:1) == 1)
        {
            btValue |= 1<<i;
        }
        close(fd);
    }

    return btValue;
}

void RepeatComm::Run()
{
    while (m_bRunFlag)
    {
        //DEBUG_HEX((char *)m_data_switch_status, 9);
        //DEBUG_HEX((char *)m_data_sen_status, 14);
        // TODO 根据 queue WaitPopLast得到最近的数据包，如果是统一类型数据返回包，则清除队列返回结果。
        // 如果不是同一类型数据包，则丢掉数据返回包，执行WaitPopLast的命令包
        // 这个是防止空管器命令缓存后导致返回的数据包和当前命令包的不匹配
        Packet packet;
        if (m_comm.WaitResponse(packet, 200) && packet.GetType() == 0x1D)
        {
            // 空管器应答太快会导致重发
            //lib_linux::Utility::Sleep(20);
            lib_linux::Utility::Sleep(40);

            // 保护
            lib_linux::AutoLock lock(m_mutex);

            // 采集户控器开关量
            m_data_switch_status[0] = GetSwitchStatus();
            ProcessSen();

            BYTE btFunction = packet.GetFunction();
            DWORD dwCurPacketID = packet.GetPacketID();
            switch (btFunction)
            {
                case 0x43:
                case 0x44:
                    {
                        // 灯控器
                        Packet packet_ret;
                        packet.SetType(0x1E);
                        if (btFunction == 0x43)
                        {
                            // 设置命令，优先处理
                            m_isKongguanSetCmd = true;

                            if (packet.GetDataLen() != 3)
                            {
                                ERROR("light set data length error");
                                // 退出
                                break;
                            }

                            packet.SetFunction(0x81);
                        }
                        else
                        {
                            if (packet.GetDataLen() > 0)
                            {
                                ERROR("light query data length error");
                                // 退出
                                break;
                            }

                            packet.SetFunction(0x01);
                        }
                        packet.ReCheckSUM();

                        DEBUG("repeat from kongguan");
                        if (m_method.transform_by_kongguan(packet, packet_ret))
                        {
                            // 灯控器空管器协议里面所有字段都不留空间
                            Packet packet_hack;
                            BYTE data[10];
                            memset(data, 0, sizeof(data));
                            // 开关状态
                            BYTE switch_status = packet_ret.GetData()[0];
                            BYTE detect_status = packet_ret.GetData()[1];
                            BYTE fault_status = packet_ret.GetData()[2];

                            // 保存开关量
                            BYTE btDestAddr = packet_ret.GetDestAddr();
                            if (btDestAddr > 0)
                            {
                                m_data_switch_status[btDestAddr] = detect_status;
                                ProcessSen();
                            }

                            data[0] = switch_status;
                            data[0] = SET_BITS(data[0], 6, 7, GET_BITS(detect_status, 0, 1));
                            // fix light version
                            data[1] = GET_BITS(detect_status, 2, 3);
                            data[1] = SET_BITS(data[1], 2, 7, GET_BITS(fault_status, 0, 5));
                            memcpy(&data[2], &(packet_ret.GetData()[3]), 8);

                            // set back to kongguang type
                            packet_hack.BuildPacket(packet_ret.GetSrcAddr(), packet_ret.GetDestAddr(),
                                    0x1D, btFunction, data, sizeof(data));
                            m_comm.SendResponseFrame(dwCurPacketID, packet_hack);
                        }
                    }
                    break;
                case 0x45:
                case 0x46:
                    {
                        // 帘控器
                        Packet packet_ret;
                        packet.SetType(0x19);
                        if (btFunction == 0x45)
                        {
                            // 设置命令，优先处理
                            m_isKongguanSetCmd = true;

                            if (packet.GetDataLen() != 2)
                            {
                                ERROR("curtain set data lenght error");
                                // 退出
                                break;
                            }

                            packet.SetFunction(0x81);
                            BYTE *pData = packet.GetData();
                            // 有无人
                            bool bHasOne = IsHasManByCurtain(packet.GetDestAddr());
                            pData[0] = SET_BIT(pData[0], 4, bHasOne?1:0);
                            pData[0] = SET_BIT(pData[0], 5, bHasOne?0:1);

                            // 季节和阳光
                            BYTE btHasSun = IsCurtainHasSunshine(packet.GetDestAddr())?1:0;
                            pData[0] = SET_BIT(pData[0], 6, m_btSeason);
                            pData[0] = SET_BIT(pData[0], 7, btHasSun);

                            // 室外温度默认25
                            pData[1] = ::round(m_outside_temp*2+50);
                        }
                        else
                        {
                            if (packet.GetDataLen() > 0)
                            {
                                ERROR("curtain query data length error");
                                // 退出
                                break;
                            }

                            packet.SetFunction(0x01);
                        }
                        packet.ReCheckSUM();

                        DEBUG("repeat from kongguan");
                        if (m_method.transform_by_kongguan(packet, packet_ret))
                        {
                            Packet packet_hack;
                            // 窗帘查询户控器多了一个字节，二空管器是老协议
                            // set back to kongguang type
                            packet_hack.BuildPacket(packet_ret.GetSrcAddr(), packet_ret.GetDestAddr(),
                                    0x1D, btFunction, packet_ret.GetData(), packet_ret.GetDataLen()-1);
                            m_comm.SendResponseFrame(dwCurPacketID, packet_hack);

                            // 查询时同步有无人
                            //if (type == TYPE_CURTAIN)
                            //{
                            //    bool bHasOne = IsHasManByCurtain(packet.GetDestAddr());
                            //    DEBUG("curtain bHasOne:%d", bHasOne);
                            //}
                            //else if (type == TYPE_AIRCONDITION)
                            //{
                            //    bool bHasOne = IsHasManByXinfeng(packet.GetDestAddr());
                            //    DEBUG("xinfeng bHasOne:%d", bHasOne);
                            //}
                            if (btFunction == 0x46)
                            {
                                // CHECK LEN
                                if (packet_ret.GetDataLen() == 6)
                                {
                                    bool bHasOne = IsHasManByCurtain(packet_ret.GetDestAddr());
                                    bool bSunshine = IsCurtainHasSunshine(packet_ret.GetDestAddr());
                                    BYTE *pData = packet_ret.GetData();
                                    bool bResponseHasOne = GET_BIT(pData[4], 0) == 1;
                                    BYTE btResponseSeason = GET_BIT(pData[4], 4);
                                    bool bResponseSunshine = GET_BIT(pData[4], 5) == 1;

                                    //BYTE btResponseOutside_temp = (pData[5]-50)/2.0;
                                    BYTE btResponseOutside_temp = pData[5];
                                    double outside_temp = m_outside_temp;
                                    // 窗帘能处理的温度范围(-25 ~ 40)
                                    if (outside_temp > 40)
                                    {
                                        outside_temp = 40;
                                    }
                                    else if (outside_temp < -25)
                                    {
                                        outside_temp = -25;
                                    }
                                    BYTE btOutside_temp = ::round(outside_temp*2+50);

                                    // 如果室外环境变了，需要立即设置窗帘参数
                                    if (bHasOne != bResponseHasOne ||
                                        m_btSeason != btResponseSeason ||
                                        bSunshine != bResponseSunshine ||
                                        btOutside_temp != btResponseOutside_temp)
                                    {
                                        DEBUG("curtain bHasOne:%d old_HasOne:%d season:%d old_season:%d sunshine:%d old_sunshine:%d \
                                               outside_temp:%.1f old_outside_temp:%.1f", 
                                                bHasOne, bResponseHasOne,
                                                m_btSeason, btResponseSeason, 
                                                bSunshine, bResponseSunshine,
                                                outside_temp, (btResponseOutside_temp-50)/2.0);
                                        try{
                                            m_method.curtain(packet_ret.GetDestAddr(), ProtocolMethod::NONE,  ProtocolMethod::NONE,
                                                    bHasOne, m_btSeason, bSunshine, outside_temp);
                                        }
                                        catch (const char *pstrErr)
                                        {
                                            ERROR("set curtain setting fail: %s", pstrErr);
                                        }
                                    }
                                }
                                else
                                {
                                    ERROR("curtain response wrong length");
                                }
                            }
                        }
                    }
                    break;
                case 0x47:
                case 0x48:
                    {
                        // 是否有人
                        BYTE btIsHasHuman = IsHasManByXinfeng(packet.GetDestAddr())?1:0;

                        // 新风
                        Packet packet_ret;
                        packet.SetType(0x1C);
                        if (btFunction == 0x47)
                        {
                            // 设置命令，优先处理
                            m_isKongguanSetCmd = true;

                            //设置
                            packet.SetFunction(0x81);
                            BYTE *pData = packet.GetData();
                            // 如果空管器设置有人，必需设置有人，风阀才会开
                            BYTE btSetHasHuman = GET_BIT(pData[0], 4);
                            if (btSetHasHuman)
                            {
                                pData[0] = SET_BIT(pData[0], 4, 1);
                            }
                            else
                            {
                                pData[0] = SET_BIT(pData[0], 4, btIsHasHuman);
                            }

                            // 保存设定温度到m_data_setting
                            m_data_setting[11] = pData[2];

                            ////设置人体感应默认1
                            //pData[0] |= 0b00010000;
                        }
                        else
                        {
                            BYTE btTemp = packet.GetData()[0];

                            //查询
                            packet.SetFunction(0x01);
                            BYTE btData[2] = {0, btTemp};
                            // 有人\无人
                            btData[0] = SET_BIT(btData[0], 4, btIsHasHuman);

                            //重新打包符合户控器到485协议
                            packet.BuildPacket(packet.GetSrcAddr(), packet.GetDestAddr(), 
                                    packet.GetType(), packet.GetFunction(), btData, sizeof(btData));
                        }
                        packet.ReCheckSUM();

                        DEBUG("repeat from kongguan");
                        if (m_method.transform_by_kongguan(packet, packet_ret))
                        {
                            // 只返回新风地址1的实时数据
                            if (packet_ret.GetDestAddr() == 1)
                            {
                                // 保存新风接口查询
                                unsigned int len_cp = MIN(packet_ret.GetPacketLen(), sizeof(DATA_RECORD));
                                memcpy(m_data_xinfeng, (char *)&packet_ret, len_cp);
                            }

                            // D7B4返回给空管器有无人
                            BYTE btTemp = packet_ret.GetData()[0];
                            btTemp = SET_BIT(btTemp, 4, btIsHasHuman);
                            packet_ret.GetData()[0] = btTemp;

                            Packet packet_hack;
                            // set back to kongguang type
                            packet_hack.BuildPacket(packet_ret.GetSrcAddr(), packet_ret.GetDestAddr(),
                                    0x1D, btFunction, packet_ret.GetData(), packet_ret.GetDataLen());
                            m_comm.SendResponseFrame(dwCurPacketID, packet_hack);
                        }
                    }
                    break;
                case 0x31:
                    {
                        // 双风控制部件调试
                        Packet packet_ret;
                        DEBUG("suangfeng part debug");
                        if (m_method.transform_by_kongguan(packet, packet_ret))
                        {
                            m_comm.SendResponseFrame(dwCurPacketID, packet_ret);
                        }
                    }
                    break;
                case 0x49:
                    {
                        if (packet.GetDataLen() > 0)
                        {
                            ERROR("meter query data length error");
                            break;
                        }

                        // 抄表接口
                        //btOriginType = 0x1F;
                        Packet packet_ret;
                        packet.SetType(0x1F);
                        packet.SetFunction(0x02);
                        packet.ReCheckSUM();

                        DEBUG("repeat from kongguan");
                        if (m_method.transform_by_kongguan(packet, packet_ret))
                        {
                            // set back to kongguang type
                            packet_ret.SetType(0x1D);
                            packet_ret.SetFunction(btFunction);
                            packet_ret.ReCheckSUM(); 
                            m_comm.SendResponseFrame(dwCurPacketID, packet_ret);
                        }
                    }
                    break;
                case 0x81:
                case 0x01:
                case 0x41:
                case 0x42:
                case 0x51:
                case 0x50:
                case 0x02:
                case 0x03:
                case 0x04:
                case 0x05:
                    // 户控器操作
                    InnerProcess(packet);
                    break;
                default:
                    break;
            }
        }
        else
        {
            // 保护
            lib_linux::AutoLock lock(m_mutex);

            // 采集户控器开关量
            m_data_switch_status[0] = GetSwitchStatus();
            ProcessSen();
        }
    }
}

void RepeatComm::InnerProcess(Packet &packet)
{
    DWORD dwCurPacketID = packet.GetPacketID();
    switch (packet.GetFunction())
    {
        case 0x81:
        case 0x01:
            {
                if (packet.GetFunction() == 0x81)
                {
                    // 保存数据
                    BYTE btFengChengJGOld = GET_BITS(m_data_setting[7], 5, 7);
                    BYTE btGereOld = GET_BITS(m_data_setting[8], 0, 2);
                    BYTE btZhaomingOld = GET_BITS(m_data_setting[8], 3, 5);
                    BYTE btAlarmCtrlOld = m_data_setting[10];
                    WORD wFengGuangOld = *(WORD *)&m_data_setting[12];
                    BYTE btXinfengOFFOld = m_data_setting[14];
                    BYTE btXinfengONOld = m_data_setting[15];
                    WORD wJiaquanAlarmOld = *(WORD *)&m_data_setting[30];
                    WORD wCO2AlarmOld = *(WORD *)&m_data_setting[32];

                    unsigned int len_cp = MIN(packet.GetPacketLen(), sizeof(DATA_RECORD));
                    memcpy(m_data_setting, (char *)&packet, len_cp);

                    // 如果设置值为无效，则不设置
                    BYTE *pPacketCur = (BYTE *)(&packet);
                    BYTE btFengChengJGCur = GET_BITS(pPacketCur[7], 5, 7);
                    BYTE btGereCur = GET_BITS(pPacketCur[8], 0, 2);
                    BYTE btZhaomingCur = GET_BITS(pPacketCur[8], 3, 5);
                    BYTE btAlarmCtrlCur = pPacketCur[10];
                    WORD wFengGuangCur = *(WORD *)&pPacketCur[12];
                    BYTE btXinfengOFFCur = pPacketCur[14];
                    BYTE btXinfengONCur = pPacketCur[15];
                    WORD wJiaquanAlarmCur = *(WORD *)&pPacketCur[30];
                    WORD wCO2AlarmCur = *(WORD *)&pPacketCur[32];

                    // 如果是无效则还原
                    if (btFengChengJGCur == 0)
                    {
                        m_data_setting[7] = SET_BITS(m_data_setting[7], 5, 7, btFengChengJGOld);
                    }

                    if (btGereCur == 0)
                    {
                        m_data_setting[8] = SET_BITS(m_data_setting[8], 0, 2, btGereOld);
                    }

                    if (btZhaomingCur == 0)
                    {
                        m_data_setting[8] = SET_BITS(m_data_setting[8], 3, 5, btZhaomingOld);
                    }
                    else if (btZhaomingCur != btZhaomingOld)
                    {
                        // 照明节能模式时间变动
                        m_isBindChange = true;
                    }

                    //if (btXinfengOFFCur != btXinfengOFFOld ||
                    //    btXinfengONCur != btXinfengONOld)
                    //{
                    //    // 新风时间模式改变
                    //    m_isBindChange = true;
                    //}

                    if (btFengChengJGOld != btFengChengJGCur ||
                        btGereOld != btGereCur ||
                        btZhaomingOld != btZhaomingCur ||
                        btAlarmCtrlOld != btAlarmCtrlCur ||
                        wFengGuangOld != wFengGuangCur ||
                        btXinfengOFFOld != btXinfengOFFCur ||
                        btXinfengONOld != btXinfengONCur ||
                        wJiaquanAlarmOld != wJiaquanAlarmCur ||
                        wCO2AlarmOld != wCO2AlarmCur)
                    {
                        INFO("Save in");
                        // 空管器设置空调制冷、制热，不保存
                        Save(DATA_SETTING, m_data_setting, packet.GetPacketLen());
                    }

                    // 火警信号
                    m_bFireAlarm = GET_BIT(m_data_setting[10], 5);

                }
                else if (packet.GetFunction() == 0x01)
                {
                    // 保存数据
                    unsigned int len_cp = MIN(packet.GetPacketLen(), sizeof(DATA_RECORD));
                    memcpy(m_data_query_setting, (char *)&packet, len_cp);
                    // 不保存工况查询，太频繁了
                    //Save(DATA_QUERY, m_data_query_setting, packet.GetPacketLen());
                    // 与设置帧同步
                    memcpy(&(m_data_setting[16]), &(m_data_query_setting[11]), 13);
                }

                ///////////////////////////////////////////////////////////////////////////
                // 空管器可能会同步设置错误的温度和模式, 在RPC同步的温度和模式前，所以现在以最近一次RPC设置为准，如果有的话。
                if (m_sync_xinfeng_temp != -1 && m_sync_xinfeng_mode != -1)
                {
                    // 人感探头控制
                    //GET_BIT(m_data_setting[10], 6)
                    // 模式（制冷和制热）0-制冷 1-制热
                    if (m_sync_xinfeng_mode == 0)
                    {
                        m_data_setting[9] = SET_BIT(m_data_setting[9], 2, 0x01);
                        m_data_setting[9] = SET_BIT(m_data_setting[9], 3, 0x00);
                    }
                    else
                    {
                        m_data_setting[9] = SET_BIT(m_data_setting[9], 2, 0x00);
                        m_data_setting[9] = SET_BIT(m_data_setting[9], 3, 0x01);
                    }

                    // 室内需求温度, 2倍后
                    m_data_setting[11] = m_sync_xinfeng_temp*2;

                    m_sync_xinfeng_temp = -1;
                    m_sync_xinfeng_mode = -1;
                }

                // 应答数据帧 D7 ~ D27
                BYTE responeData[30];
                memset(responeData, 0, sizeof(responeData));
                memset(&responeData[12], 0xFF, 27-12+1);

                // 填充数据
                responeData[7] = SET_BIT(responeData[7], 6, GET_BIT(m_data_setting[10], 6));
                responeData[7] = SET_BIT(responeData[7], 7, GET_BIT(m_data_setting[10], 7));

                responeData[8] = m_data_setting[8];

                // 粉尘间隔时间
                responeData[9] = SET_BITS(responeData[9], 0, 2, GET_BITS(m_data_setting[7], 5, 7));

                // 模式（制冷和制热）
                if (GET_BIT(m_data_setting[9], 3) == 0x01)
                {
                    responeData[9] = SET_BIT(responeData[9], 3, 0x1);
                    DEBUG("kongtiao mode hot %X", responeData[9]);
                }

                // 室内需求温度
                responeData[12] = m_data_setting[11];
                // 风管截面积
                responeData[13] = m_data_setting[12];
                responeData[14] = m_data_setting[13];

                // 室外环境参数
                ::memcpy(&responeData[15], m_data_outside_env, 11); 
                // 室外传感器
                responeData[9] = SET_BIT(responeData[9], 4, m_data_outside_sen[0]);
                responeData[9] = SET_BIT(responeData[9], 5, m_data_outside_sen[1]);
                responeData[9] = SET_BIT(responeData[9], 6, m_data_outside_sen[2]);
                responeData[9] = SET_BIT(responeData[9], 7, m_data_outside_sen[3]);
                // 火警 消防报警
                responeData[10] = SET_BIT(responeData[10], 0, m_bFireAlarm || m_bBroadcastFireAlarm);
                responeData[10] = SET_BIT(responeData[10], 1, m_data_outside_sen[4]);
                responeData[10] = SET_BIT(responeData[10], 2, m_data_outside_sen[5]);
                responeData[10] = SET_BIT(responeData[10], 3, m_data_outside_sen[6]);
                responeData[10] = SET_BIT(responeData[10], 4, m_data_outside_sen[7]);
                responeData[10] = SET_BIT(responeData[10], 5, m_data_outside_sen[8]);
                responeData[10] = SET_BIT(responeData[10], 6, m_data_outside_sen[9]);

                // 远程室内空气检测
                if (m_bInnerAirDetection)
                {
                    m_bInnerAirDetection = false;
                    responeData[11] = SET_BIT(responeData[11], 2, 1);
                }

                // 新风冷热量比值
                responeData[26] = 8;
                // 新风空调运行参数（人离阀关时间）
                responeData[27] = m_data_setting[14];

                Packet packet_ret;
                packet_ret.BuildPacket(packet.GetSrcAddr(), packet.GetDestAddr(),
                        packet.GetType(), packet.GetFunction(), 
                        &(responeData[7]), BYTE(27-7+1));

                m_comm.SendResponseFrame(dwCurPacketID, packet_ret);
            }
            break;
        case 0x41:
        case 0x42:
            {
                // 户控器接口
                if (packet.GetFunction() == 0x41)
                {
                    BYTE btRoomOld = m_data_hukong[65];
                    BYTE btFloorOld = m_data_hukong[66];

                    // 保存数据 61个数据 查询扩充到65个数据
                    unsigned int len_cp = MIN(packet.GetPacketLen(), sizeof(DATA_RECORD));
                    memcpy(m_data_hukong, (char *)&packet, len_cp);
                    //TODO 检查设置值
                    m_data_hukong[59] = MIN(m_data_hukong[59], COUNT_MAX_LIGHT);
                    m_data_hukong[60] = MIN(m_data_hukong[60], COUNT_MAX_CURTAIN);
                    m_data_hukong[61] = MIN(m_data_hukong[61], COUNT_MAX_AIRCONDITION);
                    m_data_hukong[62] = MIN(m_data_hukong[62], COUNT_MAX_METER);
                    m_data_hukong[63] = MIN(m_data_hukong[63], COUNT_MAX_HUMAN_DEV);
                    m_data_hukong[64] = MIN(m_data_hukong[64], COUNT_MAX_LEAK_DEV);
                    m_data_hukong[65] = MIN(m_data_hukong[65], MAX_ROOM);
                    m_data_hukong[66] = MIN(m_data_hukong[66], MAX_FLOOR);

                    // 楼层和地址不为0
                    if (m_data_hukong[65] == 0)
                    {
                        m_data_hukong[65] = 1;
                    }
                    if (m_data_hukong[66] == 0)
                    {
                        m_data_hukong[66] = 1;
                    }

                    if (m_data_hukong[65] != btRoomOld || m_data_hukong[66] != btFloorOld)
                    {
                        // 检查网页端是否设置可以修改 0-允许修改 1-不允许
                        if (lib_linux::Utility::system_check("_wf=`uci get iots.network.ip_change_by_web` && [ \"$_wf\" == \"0\" ]"))
                        {
                            // 允许修改
                            // 网关的子网号为楼层号转奇数，向下转(1 ~ 249), 主机号为254
                            unsigned int unNetID = m_data_hukong[66];
                            if (unNetID % 2 == 0)
                            {
                                unNetID--;
                            }

                            INFO("room and floor change:%d %d", m_data_hukong[65], m_data_hukong[66]);
                            // 设置IP地址和网关
                            char buffer[128];
                            snprintf(buffer, sizeof(buffer), "uci set network.wan.ipaddr=192.168.%d.%d", m_data_hukong[66], m_data_hukong[65]);
                            INFO("%s", buffer);
                            if (!lib_linux::Utility::system_check(buffer))
                            {
                                ERROR("set ip addr fail");
                            }

                            // DNS 255.255.0.0
                            snprintf(buffer, sizeof(buffer), "uci set network.wan.netmask=255.255.0.0");
                            INFO("%s", buffer);
                            if (!lib_linux::Utility::system_check(buffer))
                            {
                                ERROR("set dns 255.255.0.0 fail");
                            }

                            // gateway is [1/3/5...].254
                            snprintf(buffer, sizeof(buffer), "uci set network.wan.gateway=192.168.%d.254", unNetID);
                            INFO("%s", buffer);
                            if (!lib_linux::Utility::system_check(buffer))
                            {
                                ERROR("set gateway fail");
                            }

                            if (!lib_linux::Utility::system_check("uci commit"))
                            {
                                ERROR("uci commit fail");
                            }

                            // 设置立即生效
                            snprintf(buffer, sizeof(buffer), "ifconfig eth0 192.168.%d.%d netmask 255.255.0.0", m_data_hukong[66], m_data_hukong[65]);
                            //snprintf(buffer, sizeof(buffer), "ifconfig eth0 192.168.%d.%d netmask 255.255.255.0", m_data_hukong[66], m_data_hukong[65]);
                            INFO("%s", buffer);
                            if (!lib_linux::Utility::system_check(buffer))
                            {
                                ERROR("ifconfig fail");
                            }

                            // gateway is [1/3/5...].254
                            snprintf(buffer, sizeof(buffer), "route del default; route add default gw 192.168.%d.254", unNetID);
                            INFO("%s", buffer);
                            if (!lib_linux::Utility::system_check(buffer))
                            {
                                ERROR("route fail");
                            }
                        }
                        else
                        {
                            INFO("web set no modify flag!");
                        }
                    }

                    //memset(&(m_data_hukong[7]),  0b11101110, 18);
                    //memset(&(m_data_hukong[25]), 0b10011001, 28);
                    //memset(&(m_data_hukong[53]), 0b10011001, 4);
                    Save(DATA_HUKONG, m_data_hukong, packet.GetPacketLen());
                    // 设置标志位
                    m_isBindChange = true;

                    // 有可能设备数量变化
                    m_isDevChange = true;
                }

                Packet packet_ret;
                Packet *pPacket_data = (Packet *)m_data_hukong;
                BYTE btData[65];

                memcpy(btData, pPacket_data->GetData(), 52);
                // 开关量 开关量故障
                //BYTE btSwitchStatus = GetSwitchStatus();
                BYTE btSwitchStatus = m_data_switch_status[0];
                // 如果0断开，则触发开关量故障
                btSwitchStatus = SET_BITS(btSwitchStatus, 4,7, GET_BITS(~btSwitchStatus, 0, 3));

                // 如果没有关联或为人感则不报警
                BYTE btHukongBind1 = m_data_hukong[7];
                BYTE btHukongBind2 = m_data_hukong[8];
                BYTE btTemp = GET_BITS(btHukongBind1, 0, 3);
                if (GetSenType(btTemp) == TYPE_NULL || GetSenType(btTemp) == TYPE_HUMAN_GROUP)
                {
                    btSwitchStatus = SET_BIT(btSwitchStatus, 4, 0);
                }
                btTemp = GET_BITS(btHukongBind1, 4, 7);
                if (GetSenType(btTemp) == TYPE_NULL || GetSenType(btTemp) == TYPE_HUMAN_GROUP)
                {
                    btSwitchStatus = SET_BIT(btSwitchStatus, 5, 0);
                }
                btTemp = GET_BITS(btHukongBind2, 0, 3);
                if (GetSenType(btTemp) == TYPE_NULL || GetSenType(btTemp) == TYPE_HUMAN_GROUP)
                {
                    btSwitchStatus = SET_BIT(btSwitchStatus, 6, 0);
                }
                btTemp = GET_BITS(btHukongBind2, 4, 7);
                if (GetSenType(btTemp) == TYPE_NULL || GetSenType(btTemp) == TYPE_HUMAN_GROUP)
                {
                    btSwitchStatus = SET_BIT(btSwitchStatus, 7, 0);
                }

                btData[52] = btSwitchStatus;
                btData[53] = 0;
                btData[54] = m_data_hukong[59];
                btData[55] = m_data_hukong[60];
                btData[56] = m_data_hukong[61];
                btData[57] = m_data_hukong[62];
                btData[58] = m_data_hukong[63];
                btData[59] = m_data_hukong[64];
                btData[60] = m_data_hukong[65];

                // 程序版本
                btData[61] = VER_YEAR;   
                btData[62] = VER_MONTH;   
                btData[63] = VER_DAY;

                // 户控接口楼层
                btData[64] = m_data_hukong[66];

                packet_ret.BuildPacket(packet.GetSrcAddr(), packet.GetDestAddr(),
                        packet.GetType(), packet.GetFunction(), 
                        btData, sizeof(btData));
                m_comm.SendResponseFrame(dwCurPacketID, packet_ret);
            }
            break;
        case 0x51:
        case 0x50:
            {
                BYTE data_ret[7] = {0};
                data_ret[0] = m_data_others.isInternet?1:0;
                //data_ret[0] = 1;
                if (packet.GetFunction() == 0x51)
                {
                    // 时间设置有6个字节数据
                    if (packet.GetDataLen() != 6)
                    {
                        ERROR("time set data length error");
                        break;
                    }

                    // 设置时间
                    if (!m_data_others.isInternet)
                    {
                        // 如果没连网，则同步户控器时间
                        BYTE *pData = packet.GetData();
                        struct tm _tm;
                        memset(&_tm, 0, sizeof(_tm));
                        _tm.tm_sec = pData[5];
                        _tm.tm_min = pData[4];
                        _tm.tm_hour = pData[3];
                        _tm.tm_mday = pData[2];
                        _tm.tm_mon = pData[1]-1;
                        _tm.tm_year = (pData[0]+2000)-1900;

                        time_t tt = mktime(&_tm);

                        struct timeval tv;
                        tv.tv_sec = tt;
                        tv.tv_usec = 0;
                        int nRet = settimeofday(&tv, NULL);
                        if (nRet == -1)
                        {
                            ERROR("set time fail");
                        }
                    }
                }
                else
                {
                    // 0x50
                    // 查询帧应该没有数据
                    if (packet.GetDataLen() > 0)
                    {
                        ERROR("time query data length error");
                        // 退出
                        break;
                    }
                }

                // 同步抄表时间
                m_isMeterSyncTime = true;

                time_t tt = time(NULL);
                if (tt != -1)
                {
                    struct tm _tm;
                    localtime_r(&tt, &_tm);
                    data_ret[1] = (_tm.tm_year + 1900)%100;
                    data_ret[2] = _tm.tm_mon + 1;
                    data_ret[3] = _tm.tm_mday;
                    data_ret[4] = _tm.tm_hour;
                    data_ret[5] = _tm.tm_min;
                    data_ret[6] = _tm.tm_sec;
                }
                else
                {
                    // 校准失败
                    ERROR("time() return error, get time fail");
                }

                Packet packet_ret;
                packet_ret.BuildPacket(packet.GetSrcAddr(), packet.GetDestAddr(),
                        packet.GetType(), packet.GetFunction(), 
                        data_ret, sizeof(data_ret));
                m_comm.SendResponseFrame(dwCurPacketID, packet_ret);
            }
            break;
        case 0x05:
            {
                // 查询帧应该没有数据
                if (packet.GetDataLen() > 0)
                {
                    ERROR("real time query data length error");
                    // 退出
                    break;
                }

                // 实时数据查询
                Packet packet_ret;
                BYTE data[12];
                memset(data, 0, sizeof(data));

                // 送风风速 
                data[0] = m_data_xinfeng[10];
                // 送风温度
                data[1] = m_data_xinfeng[12];
                data[2] = m_data_xinfeng[13];

                try
                {
                    // 间接查询电表
                    DEBUG("realtime query call query_meter");
                    ProtocolMethod::meter_status status;
                    //if (m_method.query_meter(packet.GetDestAddr(), status))
                    // 只查询地址1的表
                    if (m_method.query_meter(1, status))
                    {
                        // 协议是小端的，MIPS是大端的
                        *(WORD *)(&data[3]) = __cpu_to_le16((WORD)(status.cur_totoal_power*100));
                        *(WORD *)(&data[5]) = __cpu_to_le16((WORD)(status.hot_cur_power*100));
                        *(WORD *)(&data[7]) = __cpu_to_le16((WORD)(status.cold_cur_rate*100));
                        *(WORD *)(&data[9]) = __cpu_to_le16((WORD)(status.gas_cur_rate*100));
                    }
                }
                catch (const char *pstrErr)
                {
                    ERROR("realtime query meter fail:%s", pstrErr);
                }

                packet_ret.BuildPacket(packet.GetSrcAddr(), packet.GetDestAddr(),
                        packet.GetType(), packet.GetFunction(), 
                        data, sizeof(data));
                m_comm.SendResponseFrame(dwCurPacketID, packet_ret);
            }
            break;
        case 0x02:
        case 0x03:
        case 0x04:
            {
                BYTE *pData = NULL;
                if (packet.GetFunction() == 0x02)
                {
                    // 空调计量
                    pData = m_data_calc_aircondition;
                }
                else if (packet.GetFunction() == 0x03)
                {
                    // 电力天然气查询
                    pData = m_data_calc_power_gas;
                }
                else if (packet.GetFunction() == 0x04)
                {
                    // 卫生热水、自来水查询
                    pData = m_data_calc_water;
                }

                Packet packet_ret;
                packet_ret.BuildPacket(packet.GetSrcAddr(), packet.GetDestAddr(),
                        packet.GetType(), packet.GetFunction(), 
                        pData, sizeof(DATA_CALC));
                m_comm.SendResponseFrame(dwCurPacketID, packet_ret);
            }
            break;
    }
}

int RepeatComm::GetSenType(BYTE code)
{
    if (code == 0)
    {
        return TYPE_NULL;
    }
    else if (code <= 8)
    {
        return TYPE_HUMAN_GROUP;
    } 
    else if (code <= 12)
    {
        return TYPE_LEAK_WATER;
    } 
    else if (code  == 13)
    {
        return TYPE_LEAK_GAS;
    } 
    else
    {
        return TYPE_NULL;
    }
}

bool RepeatComm::IsSenHumanType(BYTE code)
{
    if (code >0 && code <9)
    {
        return true;
    }
    return false;
}

void RepeatComm::ProcessSen()
{
    // 查找m_data_hukong绑定表，找出对应的开关输入的传感器，并设置状态
    // 注意！！！这里的传感器序号为序号组，可以重复，逻辑是或逻辑
    BYTE *pDataHukong = &(m_data_hukong[7]);

    // 初始化绑定表
    memset(m_data_sen_status, 0x00, sizeof(m_data_sen_status));
    for (int i=0; i<18; i++)
    {
        BYTE btCode = GET_BITS(pDataHukong[i], 0, 3);
        if (GetSenType(btCode) != TYPE_NULL)
        {
            m_data_sen_status[btCode] = m_data_sen_status[btCode] || TranSenActive(GET_BIT(m_data_switch_status[i/2], ((i%2)*2)));
            //DEBUG("i:%d code:%d %d", i, btCode, m_data_sen_status[btCode]);
        }

        btCode = GET_BITS(pDataHukong[i], 4, 7);
        if (GetSenType(btCode) != TYPE_NULL)
        {
            m_data_sen_status[btCode] = m_data_sen_status[btCode] || TranSenActive(GET_BIT(m_data_switch_status[i/2], ((i%2)*2+1)));
            //DEBUG("i:%d code:%d %d", i, btCode, m_data_sen_status[btCode]);
        }
    }
}

bool RepeatComm::IsHasManByXinfeng(BYTE addr)
{
    // 人感关联新风接口
    BYTE *pDataBind = &(m_data_hukong[53]);
    //DEBUG_HEX((char *)pDataBind, 4);
    // 是否有人
    bool bIsHasMan = false;
    // 查询户控人感关联新风控制接口
    for (int i=0; i<4; i++)
    {
        BYTE xinfeng_addr = GET_BITS(pDataBind[i], 0, 3);
        if (xinfeng_addr == addr)
        {
            // 关联了人感
            BYTE btHumanSenID = i*2+1;
            bIsHasMan = bIsHasMan || m_data_sen_status[btHumanSenID];
        }

        xinfeng_addr = GET_BITS(pDataBind[i], 4, 7);
        if (xinfeng_addr == addr)
        {
            // 关联了人感
            BYTE btHumanSenID = i*2+2;
            bIsHasMan = bIsHasMan || m_data_sen_status[btHumanSenID];
        }
    }
    //DEBUG("xinfeng id:%d %d", addr, btIsHasMan);
    return bIsHasMan;
}

bool RepeatComm::IsHasManByCurtain(BYTE addr)
{
    // 人感关联新风接口
    BYTE *pDataBind = &(m_data_hukong[49]);
    //DEBUG_HEX((char *)pDataBind, 4);
    // 查询户控人感关联窗帘
    for (int i=0; i<4; i++)
    {
        BYTE curtain_addr = i*2+1;
        if (curtain_addr == addr)
        {
            // 关联了人感
            BYTE btHumanSenID = GET_BITS(pDataBind[i], 0, 3);
            return m_data_sen_status[btHumanSenID];
        }

        curtain_addr = i*2+2;
        if (curtain_addr == addr)
        {
            // 关联了人感
            BYTE btHumanSenID = GET_BITS(pDataBind[i], 4, 7);
            return m_data_sen_status[btHumanSenID];
        }
    }
    return false;
}

int RepeatComm::RawGetDeviceCount(DevCount *pDev, int nLen)
{
    assert(nLen >= 6);
    int index = 0;
    // 户控接口设备台数表
    BYTE *pDataDevCount = &(m_data_hukong[59]);
    pDev[index].type = TYPE_LIGHT;
    pDev[index++].count = MIN(pDataDevCount[0], COUNT_MAX_LIGHT);

    pDev[index].type = TYPE_CURTAIN;
    pDev[index++].count = MIN(pDataDevCount[1], COUNT_MAX_CURTAIN);

    pDev[index].type = TYPE_AIRCONDITION;
    pDev[index++].count = MIN(pDataDevCount[2], COUNT_MAX_AIRCONDITION);

    pDev[index].type = TYPE_METER;
    pDev[index++].count = MIN(pDataDevCount[3], COUNT_MAX_METER);

    pDev[index].type = TYPE_HUMAN_GROUP;
    pDev[index++].count = MIN(pDataDevCount[4], COUNT_MAX_HUMAN_GROUP);

    return index;
}

bool RepeatComm::TranSenActive(int status)
{
    // 现在是：断开0-触发（有人/泄漏等），闭合1-不触发（没人/无泄漏)
    return !status;
}

int RepeatComm::TranSwitchStatus(bool bActive)
{
    // 现在是：触发1-断开0， 不触发0-闭合1
    return bActive?0:1;
}

int RepeatComm::TranGPIOToSwitchStatus(int gpio)
{
    // 现在是：gpio 1-断开0， gpio 0-闭合1
    return (gpio==1)?0:1;
}

bool RepeatComm::IsCurtainHasSunshine(BYTE btAddr)
{
    if (btAddr <= 8 && btAddr > 0)
    {
        WORD wCurtainData = __le16_to_cpu(*(WORD *)&m_data_hukong[57]);
        //INFO("curtain data :%04X %d %d %d", wCurtainData, m_bEastSun, m_bSouthSun, m_bWestSun);
        wCurtainData = (wCurtainData>>((btAddr-1)*2))&0x0003;
        switch (wCurtainData)
        {
            case 1:
                // 南面
                return m_bSouthSun;
            case 2:
                // 西面
                return m_bWestSun;
            case 3:
                // 东面
                return m_bEastSun;
        }
    }
    else
    {
        ERROR("IsCurtainHasSunshine invalid btAddr");
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
// 外部线程调用，需要保护
bool RepeatComm::curtain(int nDestAddr, int inner_mode, int outside_mode)
{
    bool bHasOne;
    int nSeason = 0;
    bool bSunshine = false;
    // 室外温度默认25
    double dOutsideTemp = 25;
    {
        // 保护
        lib_linux::AutoLock lock(m_mutex);
        // 有无人
        bHasOne = IsHasManByCurtain(nDestAddr);
        // 季节
        nSeason = m_btSeason;
        bSunshine = IsCurtainHasSunshine(nDestAddr);
        // 室外温度
        dOutsideTemp = m_outside_temp;
    }
    return m_method.curtain(nDestAddr, inner_mode, outside_mode,
            bHasOne, nSeason, bSunshine, dOutsideTemp);
}

bool RepeatComm::aircondition(int nDestAddr, int switch_mode, int aircondition_mode, int xinfeng_mode, int temp, int energy_save_mode, bool bFixAutoXinfengBug)
{
    bool bHasOne;
    bool bFireAlarm;
    int inner_kongguang_temp;
    {
        // 保护
        lib_linux::AutoLock lock(m_mutex);

        // 远大新风BUG， 节能模式时必须要有人开启
        if (bFixAutoXinfengBug && (switch_mode == 1))
        {
            bHasOne = true;
            m_mapFixAutoXinfengBug[nDestAddr] = 1;
            m_mapFixAutoXinfengBugCount[nDestAddr] = 0;
        }
        else
        {
            // 有无人
            bHasOne = IsHasManByXinfeng(nDestAddr);
        }

        // 室内温度，空管器温探
        inner_kongguang_temp = ((int)(m_data_query_setting[22])-50)/2;

        // 是否有火警 或 楼控中心设置了火警警报
        bFireAlarm = GET_BIT(m_data_setting[10], 5) || m_bBroadcastFireAlarm;

        //////////////////////////////////////////////////////////////////////////////
        // 更新空管器空调参数设置，否则空管器会重新设置参数, 地址1
        if (nDestAddr == 1)
        {
            // 保存新风温度和模式同步
            // 检查温度是否有效，范围是(5~35)
            if (temp > 35)
            {
                temp = 35;
            }
            else if (temp < 5)
            {
                temp = 5;
            }
            m_sync_xinfeng_temp = temp;
            m_sync_xinfeng_mode = aircondition_mode;
        }
    }

    return m_method.aircondition(nDestAddr, switch_mode, aircondition_mode, xinfeng_mode, temp,
           bHasOne, energy_save_mode, inner_kongguang_temp, bFireAlarm);
}

bool RepeatComm::query_aircondition(int nDestAddr, ProtocolMethod::aircondition_status &status, bool bSaveInBuffer)
{
    bool bHasOne;
    int inner_kongguang_temp;
    {
        // 保护
        lib_linux::AutoLock lock(m_mutex);

        // 远大新风BUG， 节能模式时必须要有人开启
        if (m_mapFixAutoXinfengBug.count(nDestAddr) > 0 && (m_mapFixAutoXinfengBug[nDestAddr] == 1))
        {
            bHasOne = true;
            if (++m_mapFixAutoXinfengBugCount[nDestAddr] >= 2)
            {
                m_mapFixAutoXinfengBug[nDestAddr] = 0;
            }
        }
        else
        {
            // 有无人
            bHasOne = IsHasManByXinfeng(nDestAddr);
        }

        // 室内温度，空管器温探
        inner_kongguang_temp = ((int)(m_data_query_setting[22])-50)/2;
    }
    return m_method.query_aircondition(nDestAddr, status, bHasOne, inner_kongguang_temp, bSaveInBuffer);
}

int RepeatComm::HumanSenToDev(BYTE btSenID, DevInfo *pInfo, int nLen)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);

    int index = 0;
    // 默认8路人感
    if (btSenID <= COUNT_MAX_HUMAN_GROUP)
    {
        // 查找人感探头对应的灯控
        BYTE *pDataLight = &(m_data_hukong[25]);
        for (int i=0; i<24; i++)
        {
            // 灯控地址和序号
            BYTE btIndex;
            BYTE btDest = i/3+1;
            // 是否绑定了当前触发的人感code
            if (GET_BITS(pDataLight[i], 0, 3) == btSenID)
            {
                btIndex = (i%3)*2 + 1;
                pInfo[index].type = TYPE_LIGHT;
                pInfo[index].addr = btDest;
                pInfo[index].opt = btIndex;
                index++;
                if (index >= nLen)
                {
                    break;
                }
            }
            if (GET_BITS(pDataLight[i], 4, 7) == btSenID)
            {
                btIndex = (i%3)*2 + 2;
                pInfo[index].type = TYPE_LIGHT;
                pInfo[index].addr = btDest;
                pInfo[index].opt = btIndex;
                index++;
                if (index >= nLen)
                {
                    break;
                }
            }
        }

        // 查找人感对应的新风
        BYTE *pDataXinfeng = &(m_data_hukong[53]);
        //DEBUG_HEX((char *)pDataXinfeng, 4);
        // 查询户控人感关联新风控制接口
        for (int i=0; i<4; i++)
        {
            if (GET_BITS(pDataXinfeng[i], 0, 3) == btSenID)
            {
                // 关联了人感
                BYTE btXinfengAddr = i*2+1;
                pInfo[index].type = TYPE_AIRCONDITION;
                pInfo[index].addr = btXinfengAddr;
                index++;
                if (index >= nLen)
                {
                    break;
                }
            }

            if (GET_BITS(pDataXinfeng[i], 4, 7) == btSenID)
            {
                // 关联了人感
                BYTE btXinfengAddr = i*2+2;
                pInfo[index].type = TYPE_AIRCONDITION;
                pInfo[index].addr = btXinfengAddr;
                index++;
                if (index >= nLen)
                {
                    break;
                }
            }
        }
    }

    DEBUG("return index:%d", index);
    return index;
}

int RepeatComm::GetDeviceCount(DevCount *pDev, int nLen)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    return RawGetDeviceCount(pDev, nLen);
}

RepeatComm::FloorRoom RepeatComm::GetFloorRoom()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    BYTE *pDataFloorRoom = &(m_data_hukong[65]);
    FloorRoom fa;
    fa.room= pDataFloorRoom[0];
    fa.floor = pDataFloorRoom[1];
    return fa;
}

bool RepeatComm::IsBindChange()
{
    bool bTemp = m_isBindChange;
    m_isBindChange = false;
    return bTemp;
}

bool RepeatComm::IsDevChange()
{
    bool bTemp = m_isDevChange;
    m_isDevChange = false;
    return bTemp;
}

int RepeatComm::GetSenStatus(BYTE *pData, int nLen)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    assert(pData && nLen >= COUNT_MAX_HUMAN_GROUP);
    // 户控接口设备台数表
    BYTE *pDataDevCount = &(m_data_hukong[59]);
    int nCount = pDataDevCount[4];
    // must check or crash !!!
    if (nCount > nLen)
    {
        nCount = nLen;
    }
    memcpy(pData,  &(m_data_sen_status[1]), nCount); 
    return nCount;
}

bool RepeatComm::SetLightOffTime(int index)
{
    if (index < 1 || index > 2)
    {
        return false;
    }

    // 保护
    lib_linux::AutoLock lock(m_mutex);
    switch (index)
    {
        case 1:
            // 10min close
            m_data_setting[8] = SET_BITS(m_data_setting[8], 3, 5, 0x03);
            break;
        case 2:
            // 2hour close
            m_data_setting[8] = SET_BITS(m_data_setting[8], 3, 5, 0x02);
            break;
    }

    // 照明节能模式时间变动
    m_isBindChange = true;
    return true;
}

int RepeatComm::QueryLightOffTime()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    BYTE btTime = GET_BITS(m_data_setting[8], 3, 5);
    switch (btTime)
    {
        case 0x03:
            return 1;
        case 0x02:
            return 2;
    }
    ERROR("QueryLightOffTime invalid time");
    return 0;
}

bool RepeatComm::SetCurtainOffTime(int index)
{
    if (index < 1 || index > 4)
    {
        return false;
    }

    // 保护
    lib_linux::AutoLock lock(m_mutex);
    switch(index)
    {
        case 1:
            // 1小时关
            m_data_setting[8] = SET_BITS(m_data_setting[8], 0, 2, 0x5);
            break;
        case 2:
            // 4小时关
            m_data_setting[8] = SET_BITS(m_data_setting[8], 0, 2, 0x2);
            break;
        case 3:
            // 8小时关
            m_data_setting[8] = SET_BITS(m_data_setting[8], 0, 2, 0x3);
            break;
        case 4:
            // 人离不关
            m_data_setting[8] = SET_BITS(m_data_setting[8], 0, 2, 0x4);
            break;
    }

    return true;
}

int RepeatComm::QueryCurtainOffTime()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    BYTE btTime = GET_BITS(m_data_setting[8], 0, 2);
    switch (btTime)
    {
        case 0x05:
            return 1;
        case 0x02:
            return 2;
        case 0x03:
            return 3;
        case 0x04:
            return 4;
    }
    ERROR("QueryCurtainOffTime invalid time");
    return 0;
}

bool RepeatComm::SetDustTime(int index)
{
    if (index < 1 || index > 4)
    {
        return false;
    }

    // 保护
    lib_linux::AutoLock lock(m_mutex);
    switch(index)
    {
        case 1:
            // 每天检测1次
            m_data_setting[7] = SET_BITS(m_data_setting[7], 5, 7, 0x3);
            break;
        case 2:
            // 每天检测2次
            m_data_setting[7] = SET_BITS(m_data_setting[7], 5, 7, 0x1);
            break;
        case 3:
            // 每天检测4次
            m_data_setting[7] = SET_BITS(m_data_setting[7], 5, 7, 0x2);
            break;
        case 4:
            // 手动启动检测
            m_data_setting[7] = SET_BITS(m_data_setting[7], 5, 7, 0x7);
            break;
    }

    return true;
}

int RepeatComm::QueryDustTime()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    BYTE btTime = GET_BITS(m_data_setting[7], 5, 7);
    switch (btTime)
    {
        case 0x03:
            return 1;
        case 0x01:
            return 2;
        case 0x02:
            return 3;
        case 0x07:
            return 4;
    }
    ERROR("QueryDustTime invalid time");
    return 0;
}

int RepeatComm::GetLightOffTime()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    BYTE btTime = GET_BITS(m_data_setting[8], 3, 5);
    if (btTime == 0x02)
    {
        // 2小时
#ifdef _TEST
        return 2*60*60;
#else
        return 2*60*60;
#endif
    }
    else if (btTime == 0x03)
    {
        // 10分钟
#ifdef _TEST
        return 10*60;
#else
        return 10*60;
#endif
    }
    else
    {
        return 0;
    }
}

RepeatComm::XinfengTime RepeatComm::GetXinfengTime()
{
    lib_linux::AutoLock lock(m_mutex);
    XinfengTime times;
    // 新风空调运行参数（人离阀关时间）
#ifdef _TEST
    times.nON = m_data_setting[14]*60;
#else
    times.nON = m_data_setting[14]*60*60;
#endif
    // 新风空调运行参数（自动开阀时间）
#ifdef _TEST
    times.nOFF = m_data_setting[15]*60;
#else
    times.nOFF = m_data_setting[15]*60;
#endif
    return times;
}

RepeatComm::air_quality RepeatComm::GetAirQuality()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    air_quality air;
    BYTE *pData = m_data_query_setting;
    // 协议是小端
    DWORD dwPM03 = 0;
    memcpy(&dwPM03, &pData[11], 3);
    air.inner_PM03 = __le32_to_cpu(dwPM03);
    air.inner_PM25 = __le16_to_cpu(*(WORD *)(&pData[14]));
    air.inner_PM10 = __le16_to_cpu(*(WORD *)(&pData[16])); 
    air.inner_VOC = (__le16_to_cpu(*(WORD *)(&pData[18])))/100.0;
    air.inner_CO2 = __le16_to_cpu(*(WORD *)(&pData[20]));
    air.inner_temp = (((int)(pData[22]))-50)/2.0;
    air.inner_humidity = pData[23];
    return air;
}

void RepeatComm::SetDeviceCount(int nLight, int nCurtain, int nMeter, int nXinfeng, bool bSave)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    BYTE *pDataDevCount = &(m_data_hukong[59]);
    pDataDevCount[0] = nLight;
    pDataDevCount[1] = nCurtain;
    pDataDevCount[2] = nXinfeng;
    pDataDevCount[3] = nMeter;

    if (bSave)
    {
        // 保存数据
        Save(DATA_HUKONG, m_data_hukong, 70);
    }

    // 设置标志位
    m_isBindChange = true;

    // 有可能设备数量变化
    m_isDevChange = true;
}

bool RepeatComm::IsMeterSyncTime()
{
    bool bTemp = m_isMeterSyncTime;
    m_isMeterSyncTime = false;
    return bTemp;
}

void RepeatComm::SetCalcAircondition(std::vector<double> &calc)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    BYTE *pData = m_data_calc_aircondition;

    // TODO 和空管器协议不一致
    *((WORD *)&pData[0]) = __cpu_to_le16(::round(calc[0]));
    *((WORD *)&pData[2]) = __cpu_to_le16(::round(calc[1]));
    *((WORD *)&pData[4]) = __cpu_to_le16(::round(calc[2]/100.0));
    *((WORD *)&pData[6]) = __cpu_to_le16(::round(calc[3]/100.0));

    // 8 9 10
    *((DWORD *)&pData[8]) = __cpu_to_le32(::round(calc[4]/100.0));
    // 11 12 13
    *((DWORD *)&pData[11]) = __cpu_to_le32(::round(calc[5]/100.0));

    *((WORD *)&pData[14]) = __cpu_to_le16(::round(calc[6]*100));
    *((WORD *)&pData[16]) = __cpu_to_le16(::round(calc[7]*100));

    *((WORD *)&pData[18]) = __cpu_to_le16(::round(calc[8]));
    *((WORD *)&pData[20]) = __cpu_to_le16(::round(calc[9]));

    // 22 23 24
    *((DWORD *)&pData[22]) = __cpu_to_le32(::round(calc[10]));
    DWORD tmp = __cpu_to_le32(::round(calc[11]));
    // 25 26 27
    memcpy(&pData[25], (BYTE *)&tmp, 3);
}

void RepeatComm::SetCalcPowerGas(std::vector<double> &calc)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    BYTE *pData = m_data_calc_power_gas;

    *((WORD *)&pData[0]) = __cpu_to_le16(::round(calc[0]*100));
    *((WORD *)&pData[2]) = __cpu_to_le16(::round(calc[1]*100));

    *((WORD *)&pData[4]) = __cpu_to_le16(::round(calc[2]));
    *((WORD *)&pData[6]) = __cpu_to_le16(::round(calc[3]));

    // 8 9 10
    *((DWORD *)&pData[8]) = __cpu_to_le32(::round(calc[4]));
    // 11 12 13
    *((DWORD *)&pData[11]) = __cpu_to_le32(::round(calc[5]));

    *((WORD *)&pData[14]) = __cpu_to_le16(::round(calc[6]*100));
    *((WORD *)&pData[16]) = __cpu_to_le16(::round(calc[7]*100));

    *((WORD *)&pData[18]) = __cpu_to_le16(::round(calc[8]));
    *((WORD *)&pData[20]) = __cpu_to_le16(::round(calc[9]));

    // 22 23 24
    *((DWORD *)&pData[22]) = __cpu_to_le32(::round(calc[10]));
    DWORD tmp = __cpu_to_le32(::round(calc[11]));
    // 25 26 27
    memcpy(&pData[25], (BYTE *)&tmp, 3);
}

void RepeatComm::SetCalcWater(std::vector<double> &calc)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    BYTE *pData = m_data_calc_water;

    *((WORD *)&pData[0]) = __cpu_to_le16(::round(calc[0]*100));
    *((WORD *)&pData[2]) = __cpu_to_le16(::round(calc[1]*100));

    *((WORD *)&pData[4]) = __cpu_to_le16(::round(calc[2]));
    *((WORD *)&pData[6]) = __cpu_to_le16(::round(calc[3]));

    // 8 9 10
    *((DWORD *)&pData[8]) = __cpu_to_le32(::round(calc[4]));
    // 11 12 13
    *((DWORD *)&pData[11]) = __cpu_to_le32(::round(calc[5]));

    *((WORD *)&pData[14]) = __cpu_to_le16(::round(calc[6]*100));
    *((WORD *)&pData[16]) = __cpu_to_le16(::round(calc[7]*100));

    *((WORD *)&pData[18]) = __cpu_to_le16(::round(calc[8]));
    *((WORD *)&pData[20]) = __cpu_to_le16(::round(calc[9]));

    // 22 23 24
    *((DWORD *)&pData[22]) = __cpu_to_le32(::round(calc[10]));
    DWORD tmp = __cpu_to_le32(::round(calc[11]));
    // 25 26 27
    memcpy(&pData[25], (BYTE *)&tmp, 3);
}

void RepeatComm::SetPrice(std::vector<double> &price)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    // 空调计量
    *((WORD *)&m_data_calc_aircondition[28]) = __cpu_to_le16(::round(price[0]*100));
    *((WORD *)&m_data_calc_aircondition[30]) = __cpu_to_le16(::round(price[1]*100));
    // 电力天然气查询
    *((WORD *)&m_data_calc_power_gas[28]) = __cpu_to_le16(::round(price[2]*100));
    *((WORD *)&m_data_calc_power_gas[30]) = __cpu_to_le16(::round(price[3]*100));
    // 卫生热水、自来水查询
    *((WORD *)&m_data_calc_water[28]) = __cpu_to_le16(::round(price[4]*100));
    *((WORD *)&m_data_calc_water[30]) = __cpu_to_le16(::round(price[5]*100));
}

RepeatComm::xinfeng_windspeed RepeatComm::GetXinfengWindSpeed()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    RepeatComm::xinfeng_windspeed speed;

    WORD wValue = __le16_to_cpu(*(WORD *)&(m_data_xinfeng[10]));
    speed.windspeed = wValue/10.0;

    // 风管截面积
    WORD wArea = __le16_to_cpu(*(WORD *)&(m_data_setting[12]));
    speed.area = wArea/1000.0;
    return speed;
}

void RepeatComm::SetXinfengWindarea(double area)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    // 风管截面积
    WORD wArea = area*1000;
    *(WORD *)&(m_data_setting[12]) = __cpu_to_le16(wArea);
}

void RepeatComm::SetOutsideEnv(std::vector<double> &data)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    assert(data.size() == 6);
    *((DWORD *)&m_data_outside_env[0]) = __cpu_to_le32(::round(data[0]));
    *((WORD *)&m_data_outside_env[3]) = __cpu_to_le16(::round(data[1]));
    *((WORD *)&m_data_outside_env[5]) = __cpu_to_le16(::round(data[2]));
    m_outside_temp = data[3];
    m_data_outside_env[7] = ::round(m_outside_temp*2+100);
    m_data_outside_env[8] = ::round(data[4]);
    *((WORD *)&m_data_outside_env[9]) = __cpu_to_le16(::round(data[5]));
}

void RepeatComm::SetOutsideSen(std::vector<int> &data)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    assert(data.size() == 10);
    std::copy(data.begin(), data.end(), m_data_outside_sen);

    m_btSeason = data[0];
    // 东面阳光信号
    m_bEastSun = data[1]==1;
    // 南面阳光信号
    m_bSouthSun = data[2]==1;
    // 西面阳光信号
    m_bWestSun = data[3]==1;

    DEBUG("setoutsidesen season:%d eastsun:%d southsun:%d westsun:%d", m_btSeason, m_bEastSun, m_bSouthSun, m_bWestSun);
}

void RepeatComm::SetInnerAirDetection()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    m_bInnerAirDetection = true;
}

void RepeatComm::SetFireAlarm(bool bAlarm)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    m_bBroadcastFireAlarm = bAlarm;
}

void RepeatComm::SetTimeSynInternet(bool bInternet)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    m_data_others.isInternet = bInternet;
    Save(DATA_OTHERS, (BYTE *)&m_data_others, sizeof(m_data_others));
}

bool RepeatComm::GetTimeSynInternet()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    return m_data_others.isInternet;
}

bool RepeatComm::SetTime(int year, int month, int day, int hour, int minute, int second)
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    
    // internet time
    m_data_others.isInternet = true;

    // 设置时间
    struct tm _tm;
    memset(&_tm, 0, sizeof(_tm));
    _tm.tm_sec = second;
    _tm.tm_min = minute;
    _tm.tm_hour = hour;
    _tm.tm_mday = day;
    _tm.tm_mon = month-1;
    _tm.tm_year = year-1900;

    time_t tt = mktime(&_tm);
    if (tt == -1)
    {
        return false;
    }

    struct timeval tv;
    tv.tv_sec = tt;
    tv.tv_usec = 0;
    int nRet = settimeofday(&tv, NULL);
    if (nRet == -1)
    {
        ERROR("settime fail");
        return false;
    }

    return true;
}

RepeatComm::XinfengTimeCtrl RepeatComm::QueryXinfengTimeCtrl()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    XinfengTimeCtrl time;
    time.nLeave = m_data_setting[14];
    time.nAutoRun = m_data_setting[15];
    return time;
}

bool RepeatComm::SetXinfengTimeCtrl(XinfengTimeCtrl &time)
{
    if (time.nLeave >= 4 && time.nLeave <= 24
        && time.nAutoRun >= 10 && time.nAutoRun <=60)
    {
        // 保护
        lib_linux::AutoLock lock(m_mutex);
        m_data_setting[14] = time.nLeave;
        m_data_setting[15] = time.nAutoRun;

        // 新风时间改变
        //m_isBindChange = true;
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////
// 报警数据
std::vector<int> RepeatComm::QueryOutsideAlarm()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    std::vector<int> vec_alarms;
    for (int i=0; i<6; i++)
    {
        vec_alarms.push_back(m_data_outside_sen[i+4]);
    }

    return vec_alarms;
}

std::vector<int> RepeatComm::QueryInnerAlarm()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    std::vector<int> vec_alarms;
    vec_alarms.push_back(GET_BIT(m_data_setting[10], 0));
    vec_alarms.push_back(GET_BIT(m_data_setting[10], 1));
    vec_alarms.push_back(GET_BIT(m_data_setting[10], 2));
    vec_alarms.push_back(GET_BIT(m_data_setting[10], 3));
    vec_alarms.push_back(GET_BIT(m_data_setting[10], 4));

    //INFO("m_data_sen_status 9-12:%d %d %d %d 13:%d", m_data_sen_status[9], m_data_sen_status[10], m_data_sen_status[11], m_data_sen_status[12], m_data_sen_status[13]);
    // 9 - 12 漏水探测
    int leak_water = m_data_sen_status[9] + m_data_sen_status[10] + m_data_sen_status[11] + m_data_sen_status[12];
    vec_alarms.push_back(int(leak_water>0));
    // 13 - 燃气泄露
    vec_alarms.push_back(m_data_sen_status[13]);

    //// 火警
    //vec_alarms.push_back(GET_BIT(m_data_setting[10], 5));
    //// 新风阀故障
    //vec_alarms.push_back(GET_BIT(m_data_xinfeng[14], 0));
    //// 调风阀故障
    //vec_alarms.push_back(GET_BIT(m_data_xinfeng[14], 1));
    //// 排风阀故障
    //vec_alarms.push_back(GET_BIT(m_data_xinfeng[14], 2));
    return vec_alarms;
}

std::vector<int> RepeatComm::QueryInnerAlarmNew()
{
    // 保护
    lib_linux::AutoLock lock(m_mutex);
    std::vector<int> vec_alarms;
    vec_alarms.push_back(GET_BIT(m_data_setting[10], 0));
    vec_alarms.push_back(GET_BIT(m_data_setting[10], 1));
    vec_alarms.push_back(GET_BIT(m_data_setting[10], 2));
    vec_alarms.push_back(GET_BIT(m_data_setting[10], 3));
    vec_alarms.push_back(GET_BIT(m_data_setting[10], 4));

    //INFO("m_data_sen_status 9-12:%d %d %d %d 13:%d", m_data_sen_status[9], m_data_sen_status[10], m_data_sen_status[11], m_data_sen_status[12], m_data_sen_status[13]);
    // 9 - 12 漏水探测
    int leak_water = m_data_sen_status[9] + m_data_sen_status[10] + m_data_sen_status[11] + m_data_sen_status[12];
    vec_alarms.push_back(int(leak_water>0));
    // 13 - 燃气泄露
    vec_alarms.push_back(m_data_sen_status[13]);

    // 火警
    vec_alarms.push_back(GET_BIT(m_data_setting[10], 5) || m_bBroadcastFireAlarm);
    // 新风阀故障
    vec_alarms.push_back(GET_BIT(m_data_xinfeng[14], 0));
    // 调风阀故障
    vec_alarms.push_back(GET_BIT(m_data_xinfeng[14], 1));
    // 排风阀故障
    vec_alarms.push_back(GET_BIT(m_data_xinfeng[14], 2));
    // 风量计故障
    vec_alarms.push_back(GET_BIT(m_data_xinfeng[14], 3));
    // 温度传感器故障
    vec_alarms.push_back(GET_BIT(m_data_xinfeng[14], 4));
    // 风量计通信故障
    vec_alarms.push_back(GET_BIT(m_data_xinfeng[14], 5));
    // 新风阀通信故障
    vec_alarms.push_back(GET_BIT(m_data_xinfeng[14], 6));
    // 调风阀通信故障
    vec_alarms.push_back(GET_BIT(m_data_xinfeng[14], 7));
    // 排风阀通信故障
    vec_alarms.push_back(GET_BIT(m_data_xinfeng[15], 0));
    return vec_alarms;
}
