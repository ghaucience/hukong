#include "protocol_method.h"
#include <algorithm>
#include <cmath>

ProtocolMethod::ProtocolMethod()
{
}

unsigned int ProtocolMethod::BCD(const BYTE *pByte, int nLen)
{
    unsigned int unResult = 0;
    for (int i=0; i<nLen;i++) 
    {
        unResult = (unResult * 100) + ((pByte[i] >> 4) * 10 ) + (pByte[i] & 0x0F);     
    }

    return unResult;
} 

bool ProtocolMethod::WaitCheckResult(int nDestAddr, BYTE btType, BYTE btFunction, Packet &packet)
{
    if (!WaitResponse(packet))
    { ERROR("serialport wait response timeout addr:%02X type:%02X fun:%02X\n", nDestAddr, btType, btFunction); return false; }

    //if (!packet.IsReturnOK())
    if (packet.GetDestAddr() != nDestAddr ||
        packet.GetType() != btType ||
        packet.GetFunction() != btFunction)
    {
        ERROR("serialport cmd response fail dest:%02X type:%02X function:%02X\n",
              packet.GetDestAddr(),
              packet.GetType(),
              packet.GetFunction());
        return false;
    }
    return true;
}

bool ProtocolMethod::light_switch(int nDestAddr, int nIndex, int mode)
{
    if(!((mode == ON || mode == OFF) &&
                (nDestAddr >= 1 && nDestAddr <= 8) &&
                (nIndex >= 0 && nIndex <= 6)))
    {
        DEBUG("throw param invalid %d", nDestAddr);
        throw "param invalid";
    }

    BYTE data[3] = {0, 0, 0};
    if (mode == ON)
    {
        if (nIndex == 0)
        {
            memset(data, 0x55, sizeof(data));
        }
        else
        {
            data[0] = 1 << (nIndex -1)*2;
            data[1] = 1 << ((nIndex -1)*2 - 8);
            data[2] = 1 << ((nIndex -1)*2 - 16);
        }
    }
    else
    {
        if (nIndex == 0)
        {
            memset(data, 0xAA, sizeof(data));
        }
        else
        {
            data[0] = 1 << ((nIndex -1)*2+1);
            data[1] = 1 << ((nIndex -1)*2+1 - 8);
            data[2] = 1 << ((nIndex -1)*2+1 - 16);
        }
    }

    Packet packet;
    return transform(0x00, nDestAddr, 0x1E, 0x81, data, sizeof(data), packet);
}

bool ProtocolMethod::query_light_switch(int nDestAddr, light_switch_status &status, bool bSaveInBuffer)
{
    if (!(nDestAddr >= 1 && nDestAddr <= 8))
    {
        DEBUG("throw param invalid %d", nDestAddr);
        throw "param invalid";
    }

    // 不重发，太占总线了
    Packet packet;
    //if (transform(0x00, nDestAddr, 0x1E, 0x01, NULL, 0, packet, bSaveInBuffer))
    if (transform(0x00, nDestAddr, 0x1E, 0x01, NULL, 0, packet, bSaveInBuffer, true))
    {
        //assert(packet.GetDataLen() == 11);
        BYTE *pData = (BYTE *)packet.GetData();
        memset(&status, 0, sizeof(status));
        status.light1_status = GET_BIT(pData[0], 0);
        status.light2_status = GET_BIT(pData[0], 1);
        status.light3_status = GET_BIT(pData[0], 2);
        status.light4_status = GET_BIT(pData[0], 3);
        status.light5_status = GET_BIT(pData[0], 4);
        status.light6_status = GET_BIT(pData[0], 5);
        return true;
    }
    else
    {
        return false;
    }
}

bool ProtocolMethod::curtain(int nDestAddr, int inner_mode, int outside_mode, 
        bool bHasOne, int nSeason, bool bSunshine, double dOutsideTemp)
{
    if (!((nDestAddr >= 1 && nDestAddr <= 8) &&
                (inner_mode >= 0 && inner_mode <= 3) &&
                (outside_mode >= 0 && outside_mode <= 3)))
    {
        DEBUG("throw param invalid %d", nDestAddr);
        throw "param invalid";
    }

    BYTE cmds[4] = {2, 1, 3, 0};
    BYTE data[2] = {0, 0};
    BYTE cmd_inner = cmds[inner_mode];
    BYTE cmd_outside = cmds[outside_mode];

    data[0] |= cmd_outside&0x03;
    data[0] <<= 2;
    data[0] |= cmd_inner&0x03;

    // 无人
    data[0] = SET_BIT(data[0], 4, (bHasOne?1:0));
    data[0] = SET_BIT(data[0], 5, (bHasOne?0:1));

    // 季节和无阳光
    data[0] = SET_BIT(data[0], 6, nSeason);
    data[0] = SET_BIT(data[0], 7, (bSunshine?1:0));

    // 室外温度
    data[1] = ::round(dOutsideTemp*2+50);

    Packet packet;
    return transform(0x00, nDestAddr, 0x19, 0x81, data, sizeof(data), packet);
}

bool ProtocolMethod::query_curtain(int nDestAddr, curtain_status &status, bool bSaveInBuffer)
{
    if (!(nDestAddr >= 1 && nDestAddr <= 8))
    {
        DEBUG("throw param invalid %d", nDestAddr);
        throw "param invalid";
    }

    // 不重发，太占总线了
    Packet packet;
    //if (transform(0x00, nDestAddr, 0x19, 0x01, NULL, 0, packet, bSaveInBuffer))
    if (transform(0x00, nDestAddr, 0x19, 0x01, NULL, 0, packet, bSaveInBuffer, true))
    {
        //assert(packet.GetDataLen() == 6);
        memset(&status, 0, sizeof(status));
        BYTE btStatus = packet.GetData()[0];
        BYTE btSetting = packet.GetData()[4];
        BYTE btOutsideTemp = packet.GetData()[5];

        // translate status code from real
        BYTE status_code[] = {-1, 1, 0, 2, -1, 3, 4};
        status.inner = status_code[btStatus & 0b00000111];
        status.outside = status_code[(btStatus & 0b00111000)>>3];
        status.inner_fault = (btStatus & 0b01000000) >> 6;
        status.outside_fault = (btStatus & 0b10000000) >> 7;

        // 有无人
        status.has_man = GET_BIT(btSetting, 0);
        // 0 - 手动控制
        // 1 - 自动控制
        status.control = GET_BIT(btSetting, 3);
        // 0 - 夏季
        // 1 - 冬季
        status.season = GET_BIT(btSetting, 4);
        // 0 - 无
        // 1 - 有
        status.sunshine = GET_BIT(btSetting, 5);
        // 室外温度
        status.outside_temp = (btOutsideTemp-50)/2.0;
        return true;
    }
    else
    {
        return false;
    }
}

bool ProtocolMethod::time_syn_meter(int nDestAddr)
{
    if (!(nDestAddr >= 1 && nDestAddr <= 4))
    {
        DEBUG("throw param invalid %d", nDestAddr);
        throw "param invalid";
    }

    BYTE data[6];
    time_t tt = time(NULL);
    if (tt != -1)
    {
        struct tm _tm;
        localtime_r(&tt, &_tm);
        data[0] = (_tm.tm_year + 1900)%100;
        data[1] = _tm.tm_mon + 1;
        data[2] = _tm.tm_mday;
        data[3] = _tm.tm_hour;
        data[4] = _tm.tm_min;
        data[5] = _tm.tm_sec;

        Packet packet;
        return transform(0x00, nDestAddr, 0x1F, 0x51, data, sizeof(data), packet);
    }
    else
    {
        // 校准失败
        ERROR("time() return error in time_syn_meter, get time fail");
    }
    return false;
}

bool ProtocolMethod::query_meter(int nDestAddr, meter_status &status, bool bSaveInBuffer)
{
    if (!(nDestAddr >= 1 && nDestAddr <= 4))
    {
        DEBUG("throw param invalid %d", nDestAddr);
        throw "param invalid";
    }

    // 不重发，太占总线了
    Packet packet;
    //if (transform(0x00, nDestAddr, 0x1F, 0x02, NULL, 0, packet, bSaveInBuffer))
    if (transform(0x00, nDestAddr, 0x1F, 0x02, NULL, 0, packet, bSaveInBuffer, true))
    {
        //assert(packet.GetDataLen() == 51);
        memset(&status, 0, sizeof(meter_status));

        BYTE btUnit;
        BYTE *pData = packet.GetData();

        // 反转
        std::reverse(pData, pData+5);
        std::reverse(&pData[5], &pData[5]+5);
        std::reverse(&pData[10], &pData[10]+5);
        std::reverse(&pData[15], &pData[15]+5);
        std::reverse(&pData[20], &pData[20]+3);
        std::reverse(&pData[23], &pData[23]+3);
        std::reverse(&pData[26], &pData[26]+5);
        std::reverse(&pData[31], &pData[31]+5);
        std::reverse(&pData[36], &pData[36]+3);
        std::reverse(&pData[39], &pData[39]+4);
        std::reverse(&pData[43], &pData[43]+4);
        std::reverse(&pData[47], &pData[47]+4);

        status.cold_cur_rate = (BCD(pData, 5)) % 100000000 / 10000.0;
        status.hot_cur_rate =  (BCD(&pData[5], 5)) % 100000000 / 10000.0;
        status.cold_totoal = (BCD(&pData[10], 5)) % 100000000 / 100.0;
        status.hot_totoal = (BCD(&pData[15], 5)) % 100000000/ 100.0;
        status.hot_input_temp = (BCD(&pData[20], 3)) / 100.0;
        status.hot_output_temp = (BCD(&pData[23], 3)) / 100.0;
        // 热水累计热量
        status.hot_totoal_degree = (BCD(&pData[26], 5)) % 100000000 / 100.0;
        // 默认单位是 kWh
        btUnit = pData[26];
        if (btUnit == 0x08)
        {
            // MWh
            status.hot_totoal_degree *= 1000;
        }
        else if (btUnit == 0x0A)
        {
            // MWh * 100
            status.hot_totoal_degree *= 100000;
        }

        // 热水瞬时热量
        status.hot_cur_power = (BCD(&pData[31], 5)) % 100000000 / 100.0;
        // 默认单位是kW
        btUnit = pData[31];
        if (btUnit == 0x14)
        {
            // W
            status.hot_cur_power /= 1000.0;
        }
        else if (btUnit == 0x1A)
        {
            // MW
            status.hot_cur_power *= 1000;
        }

        status.cur_totoal_power = (BCD(&pData[36], 3)) / 10000.0;
        status.forward_totoal_degree = (BCD(&pData[39], 4)) / 100.0;
        status.gas_cur_rate = (BCD(&pData[43], 4)) / 10000.0;
        status.gas_totoal_rate = (BCD(&pData[47], 4));
        return true;
    }
    else
    {
        return false;
    }
}

bool ProtocolMethod::aircondition(int nDestAddr, int switch_mode, int aircondition_mode, int xinfeng_mode, int temp,
        bool bHasOne, int energy_save_mode, int inner_kongguang_temp, bool bFireAlarm)
{
    if (!((nDestAddr >= 1 && nDestAddr <= 8) &&
          (switch_mode == ON || switch_mode == OFF || switch_mode == NONE) &&
          (aircondition_mode == MODE_COLD || aircondition_mode == MODE_HOT) &&
          (xinfeng_mode >= 1 && xinfeng_mode <= 3) &&
          (temp >= 5 && temp <= 35)))
    {
        DEBUG("throw param invalid %d", nDestAddr);
        throw "param invalid";
    }

    BYTE data[6];
    memset(data, 0, sizeof(data));
    // ON/OFF/NONE
    if (switch_mode != NONE)
    {
        data[0] |= ((switch_mode == ON)?0x01:0x02);
    }
    // 新风+空调
    data[0] = SET_BITS(data[0], 2, 3, xinfeng_mode);
    // 有人无人
    data[0] = SET_BIT(data[0], 4, (bHasOne?1:0));
    // 新风空调节能模式
    data[0] = SET_BITS(data[0], 5, 7, energy_save_mode);

    // 冷热模式
    data[1] = SET_BIT(data[1], 0, aircondition_mode);
    // 人体感应开关，必须有效
    data[1] = SET_BIT(data[1], 1, 1);
    // 火警
    data[1] = SET_BIT(data[1], 2, (bFireAlarm?1:0));

    // 温度
    data[2] = temp*2;
    // 控管器温度 
    data[3] = inner_kongguang_temp*2 + 50;

    Packet packet;

    // 新风应答有点卡，延时10ms
    lib_linux::Utility::Sleep(20);
    return transform(0x00, nDestAddr, 0x1C, 0x81, data, sizeof(data), packet);
}

bool ProtocolMethod::query_aircondition(int nDestAddr, aircondition_status &status,
        bool bHasOne, int inner_kongguang_temp, bool bSaveInBuffer)
{
    if (!((nDestAddr >= 1 && nDestAddr <= 8)))
    {
        DEBUG("throw param invalid %d", nDestAddr);
        throw "param invalid";
    }

    BYTE data[2];
    ::memset(data, 0, sizeof(data));

    // 有人无人
    data[0] = SET_BIT(data[0], 4, (bHasOne?1:0));
    // 空管器温度
    data[1] = inner_kongguang_temp*2 + 50;

    // 新风应答有点卡，延时10ms
    lib_linux::Utility::Sleep(20);

    // 不重发，太占总线了
    Packet packet;
    //if (transform(0x00, nDestAddr, 0x1C, 0x01, &data, sizeof(data), packet, bSaveInBuffer))
    if (transform(0x00, nDestAddr, 0x1C, 0x01, &data, sizeof(data), packet, bSaveInBuffer, true))
    {
        //assert(packet.GetDataLen() == 14);
        memset(&status, 0, sizeof(status));
        BYTE *pData = packet.GetData();
        status.switch_mode = pData[0] & 0x01;
        status.xinfeng_mode = GET_BITS(pData[0], 2, 3);
        status.energy_save_mode = GET_BITS(pData[0], 5, 7);

        status.aircondition_mode = pData[1] & 0x01;
        status.temp_setting = pData[9]/2;

        status.nXinfengSwitch = GET_BIT(pData[2], 0);
        status.nTiaofengSwitch = GET_BIT(pData[2], 1);
        status.nPaifengSwitch = GET_BIT(pData[2], 2);

        // 火警
        status.bFireAlarm = GET_BIT(pData[1], 2);
        // 其他警报
        status.btAlarm1 = pData[7];
        status.btAlarm2 = pData[8];
        return true;
    }
    else
    {
        return false;
    }
}

bool ProtocolMethod::light_upgrade(int nDestAddr)
{
    Packet packet;
    return transform(0x00, nDestAddr, 0x1E, 0x53, NULL, 0, packet);
}

bool ProtocolMethod::light_type(int nDestAddr, int &type)
{
    HD_TYPE t;
    if (query_type(nDestAddr, 0x1E, t))
    {
        type = t.type;
        return true;
    }
    else
    {
        return false;
    }
}

bool ProtocolMethod::curtain_upgrade(int nDestAddr)
{
    Packet packet;
    return transform(0x00, nDestAddr, 0x19, 0x53, NULL, 0, packet);
}

bool ProtocolMethod::curtain_type(int nDestAddr, int &type)
{
    HD_TYPE t;
    if (query_type(nDestAddr, 0x19, t))
    {
        type = t.type;
        return true;
    }
    else
    {
        return false;
    }
}

bool ProtocolMethod::query_type(int nDestAddr, int typeCode, HD_TYPE &type)
{
    ::memset(&type, 0, sizeof(type));
    Packet packet;
    if (transform(0x00, nDestAddr, typeCode, 0x5A, NULL, 0, packet))
    {
        switch (packet.GetData()[0])
        {
            case 0x0A:
                type.type = HD_TYPE_A;
                break;
            case 0x0B:
                type.type = HD_TYPE_B;
                break;
            case 0x0C:
                type.type = HD_TYPE_C;
                break;
            case 0x0D:
                type.type = HD_TYPE_D;
                break;
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool ProtocolMethod::meter_upgrade(int nDestAddr)
{
    Packet packet;
    return transform(0x00, nDestAddr, 0x1F, 0x53, NULL, 0, packet);
}

bool ProtocolMethod::transform_by_kongguan(Packet &packet, Packet &packet_ret)
{
    // 不缓存数据，单发一次
    return transform(packet.GetSrcAddr(), packet.GetDestAddr(),
            packet.GetType(), packet.GetFunction(), packet.GetData(), packet.GetDataLen(), packet_ret, false, true);
}

bool ProtocolMethod::transform(BYTE btSrcAddr, BYTE btDestAddr,
                               BYTE btType, BYTE btFunction, void *pData, BYTE btLen, 
                               Packet &packet_ret, bool bSaveInBuffer, bool bStopResendTemp)
{
    bool bRet;
    DWORD dwPacketTypeID = Packet::GetPacketTypeID(btSrcAddr, btDestAddr, btType, btFunction);
    if (!bSaveInBuffer)
    {
        // 如果要保存应答数据到缓存，就不需要读缓存了
        // 直接从缓存取数据, 比较快速
        lib_linux::AutoLock lock(m_query_mutex);
        if (m_queryPacketMap.count(dwPacketTypeID) > 0)
        {
            // 返回缓存数据
            packet_ret = m_queryPacketMap[dwPacketTypeID];
            return true;
        }
        else
        {
            // 客户发现数据混乱-_-
            // 这些查询帧只能查缓存，不转发
            if (m_queryPacketBlockSet.count(btType<<8|btFunction) > 0)
            {
                return false;
            }
        }
    }

    {
        // 485发送, 比较耗时
        lib_linux::AutoLock lock(m_mutex);
        bool bResend = true;
        if (bStopResendTemp)
        {
            bResend = IsEnableResend();
            EnableResend(false);
        }
        bRet = SendFrame(btSrcAddr, btDestAddr, btType, btFunction, pData, btLen) &&
               WaitCheckResult(btDestAddr, btType, btFunction, packet_ret);
        if (bStopResendTemp)
        {
            EnableResend(bResend);
        }
    }

    if (bSaveInBuffer)
    {
        // 更新缓存
        lib_linux::AutoLock lock(m_query_mutex);
        // 屏蔽转发，只能查询缓存
        m_queryPacketBlockSet.insert(btType<<8|btFunction);
        if (bRet)
        {
            // 放进缓存
            m_queryPacketMap[packet_ret.GetPacketTypeID()] = packet_ret;
        }
        else
        {
            // 删除旧的数据
            m_queryPacketMap.erase(dwPacketTypeID);
        }
    }
    else
    {
        // 转换命令返回数据包到查询缓存数据包
        lib_linux::AutoLock lock(m_query_mutex);
        if (bRet)
        {
            DWORD from = btType<<8|btFunction;
            if (m_cmdPacketTransMap.count(from) > 0)
            {
                DWORD to = m_cmdPacketTransMap[from];
                BYTE btToType = (BYTE)(to>>8);
                BYTE btToFunction = (BYTE)to;
                DEBUG("cmd_to_query: %02X-%02X to %02X-%02X", btType, btFunction, btToType, btToFunction);

                Packet packet_tran = packet_ret;

                packet_tran.SetType(btToType);
                packet_tran.SetFunction(btToFunction);
                packet_tran.ReCheckSUM();

                //if (m_queryPacketMap.count(packet_tran.GetPacketTypeID()) > 0)
                //{
                //    DEBUG("cmd_to_query: find old query data");
                //}

                // 放进缓存
                m_queryPacketMap[packet_tran.GetPacketTypeID()] = packet_tran;
                DEBUG("cmd_toy_query: result");
                DEBUG_HEX((char *)&packet_tran, packet_tran.GetPacketLen());
            }
        }
    }

    return bRet;
}

void  ProtocolMethod::SetCmdPacketTransMap(BYTE from_type, BYTE from_function, BYTE to_type, BYTE to_function)
{
    // 转换命令返回数据包到查询缓存数据包
    lib_linux::AutoLock lock(m_query_mutex);

    DWORD from = from_type<<8|from_function;
    DWORD to = to_type<<8|to_function;
    assert(m_cmdPacketTransMap.count(from) == 0);
    m_cmdPacketTransMap[from] = to;
}
