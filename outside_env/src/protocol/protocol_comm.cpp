//////////////////////////////////////////////////////////////////////////////
//                          利尔达科技有限公司
//说    明：协议解析类实现
//作    者：钟柳波
//////////////////////////////////////////////////////////////////////////////
#include "protocol_comm.h"
#include "utility.h"
#include <asm/byteorder.h>

CProtocolComm::CProtocolComm()
{
}

CProtocolComm::~CProtocolComm()
{
}

void CProtocolComm::OnReceive(int nLen)
{
    char *buf = m_buffer.free_space();
    int len = m_buffer.free_size(); 

    assert( buf != 0 );

    //缓冲区是否溢出
    if( len < nLen )
    {
        m_buffer.realloc(nLen);
        buf = m_buffer.free_space();
        len = m_buffer.free_size();
    }

    //读串口数据
    int nReadLen = Read(buf,len);
    if( nReadLen == 0)
    {
        return ;
    }
    else
    {
        m_buffer.move_pos(nReadLen);
    }

    try{
        Packet packet;
        while (true)
        {
            if (m_buffer.GetPacket(packet))
            {
                //压入队列中
                m_queue.Push(packet);
            }
            else
            {
                break;
            }
        }
    }
    catch (...)
    {
        //清空数据
        m_buffer.clear();
        // flush serialport
        Flush();
    }
}

bool CProtocolComm::SendFrame(BYTE btAddr, BYTE btCmd, WORD wRegAddr, WORD wRegAddrCount, void *pData, BYTE btLen)
{
    // clear receive queue
    m_queue.Clear();

    int nLen = m_packet.BuildPacket(btAddr, btCmd, wRegAddr, wRegAddrCount, pData, btLen);
    int nSend = Write((char *)&m_packet, nLen);
    if (nSend > 0)
    {
        DEBUG("send:");
        DEBUG_HEX((char *)&m_packet, nSend);
        return true;
    }
    ERROR("SendFrame write fail");
    return false;
}

bool CProtocolComm::WaitResponse(Packet &packet, int timeoutMS)
{
    int count = RESEND_COUNT;
    while (!m_queue.WaitPop(timeoutMS, packet))
    {
        count--;
        if (count <= 0)
        {
            return false;
        }

        // resend
        int nSend = Write((char *)&m_packet, m_packet.GetPacketLen());
        if (nSend > 0)
        {
            WARNING("resend cmd:0x%02X ", m_packet.GetCmd());
            DEBUG_HEX((char *)&m_packet, nSend);
        }
        else
        {
            ERROR("SendFrame write fail resend");
        }
    }
    return true;
}

// modbus使用大端的!!!
bool CProtocolComm::QueryEnv(env_info &info)
{
    // 485发送保护
    lib_linux::AutoLock lock(m_mutex);
    Packet packet;
    if (SendFrame(0x01, 0x03, 0x01, 0x11, NULL, 0) && WaitResponse(packet))
    {
        if (packet.GetDataLen() == 34)
        {
            BYTE *pPacketData = (BYTE *)&packet;
            ::memset(&info, 0, sizeof(info));
            info.PM03 = __le32_to_cpu(*(DWORD *)&pPacketData[5]);
            info.PM25 = __le16_to_cpu(*(WORD *)&pPacketData[9]);
            info.PM10 = __le16_to_cpu(*(WORD *)&pPacketData[11]);

            info.jiaquang = __le16_to_cpu(*(WORD *)&pPacketData[13])/100.0;
            info.CO2 = __le16_to_cpu(*(WORD *)&pPacketData[15]);
            info.temp = (__le16_to_cpu(*(WORD *)&pPacketData[17])-100)/2.0;
            info.wet = __le16_to_cpu(*(WORD *)&pPacketData[19]);

            info.season = (GET_BITS(pPacketData[29], 0, 1) == 2)?1:0;
            info.bEastsun = GET_BIT(pPacketData[29], 2);
            info.bSouthsun = GET_BIT(pPacketData[29], 3);
            info.bWetsun = GET_BIT(pPacketData[29], 4);

            info.bSunsen1Fault = GET_BIT(pPacketData[30], 0);
            info.bSunsen2Fault = GET_BIT(pPacketData[30], 1);
            info.bSunsen3Fault = GET_BIT(pPacketData[30], 2);
            info.bDustsenFault = GET_BIT(pPacketData[30], 3);
            info.bWetsenFault = GET_BIT(pPacketData[30], 4);
            info.bTempsenFault = GET_BIT(pPacketData[30], 5);

            return true;
        }
    }
    return false;
}

bool CProtocolComm::SetEnv(setting_info &info)
{
    // 485发送保护
    lib_linux::AutoLock lock(m_mutex);
    Packet packet;
    BYTE btData[16];
    ::memset(btData, 0, sizeof(btData));
    *(WORD *)&btData[0] = __cpu_to_le16(info.SummerDays);
    *(WORD *)&btData[2] = __cpu_to_le16(info.SummerTemp);
    *(WORD *)&btData[4] = __cpu_to_le16(info.SummerTempContinue);
    *(WORD *)&btData[6] = __cpu_to_le16(info.WinterDays);
    *(WORD *)&btData[8] = __cpu_to_le16(info.WinterTemp);
    *(WORD *)&btData[10] = __cpu_to_le16(info.WinterTempContinue);
    *(WORD *)&btData[12] = __cpu_to_le16(info.LightStrength);
    btData[14] = SET_BIT(btData[14], 0, info.OutsideAirDetection?1:0);
    if (SendFrame(0x01, 0x10, 0x13, 0x08, btData, sizeof(btData)) && WaitResponse(packet))
    {
        return true;
    }
    return false;
}
