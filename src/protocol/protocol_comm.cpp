//////////////////////////////////////////////////////////////////////////////
//                          利尔达科技有限公司
//说    明：协议解析类实现
//作    者：钟柳波
//////////////////////////////////////////////////////////////////////////////
#include "protocol_comm.h"
#include "utility.h"

CProtocolComm::CProtocolComm()
 :m_bResend(false)
{
    m_led.Open(LED1_GPIO);
}

CProtocolComm::~CProtocolComm()
{
}

void CProtocolComm::OnReceive(int nLen)
{
    //锁住线程资源
    //lib_linux::AutoLock lock(m_mutex);
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
        m_led.Flash();
    }
}

bool CProtocolComm::SendFrame(BYTE btSrcAddr, BYTE btDestAddr,
        BYTE btType, BYTE btFunction, void *pData, BYTE btLen, bool bClearRecvQueue)
{
    if (bClearRecvQueue)
    {
        //DEBUG("clear receive queue");
        // clear receive queue
        m_queue.Clear();
    }

    int nLen = m_packet.BuildPacket(btSrcAddr, btDestAddr, btType, btFunction, pData, btLen);
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

bool CProtocolComm::SendFrame(Packet &packet)
{
    return SendFrame(packet.GetSrcAddr(), packet.GetDestAddr(),
                     packet.GetType(), packet.GetFunction(),
                     packet.GetData(), packet.GetDataLen());
}

bool CProtocolComm::SendResponseFrame(DWORD dwRequestPacketID, Packet &packet)
{
    Packet packet_remain;
    if (WaitPopLast(0, packet_remain))
    {
        WARNING("sendResponseFrame queue get test");
        // 如果是之前请求包的重发，则立即返回结果,否则丢弃结果包
        if (packet_remain.GetPacketID() != dwRequestPacketID)
        {
            // 丢弃返回包
            WARNING("sendResponseFrame queue find diff id packet");
            // 重新放入队列，等待处理
            m_queue.Push(packet_remain);
            return false;
        }
    }

    return SendFrame(packet.GetSrcAddr(), packet.GetDestAddr(),
                     packet.GetType(), packet.GetFunction(),
                     packet.GetData(), packet.GetDataLen(), false);
}

bool CProtocolComm::WaitPopLast(int nTimeoutMS, Packet &packet)
{
    if(m_queue.WaitPop(nTimeoutMS, packet))
    {
        //DEBUG("WaitPopLast queue pop packet");
        //DEBUG_HEX((char *)&packet, packet.GetPacketLen());
        // get last packet
        while (m_queue.WaitPop(0, packet))
        {
            WARNING("WaitPopLast queue pop has other packet");
            //DEBUG_HEX((char *)&packet, packet.GetPacketLen());
        }
        //DEBUG("WaitPopLast queue pop packet end");
        return true;
    }
    else
    {
        return false;
    }
}

bool CProtocolComm::WaitResponse(Packet &packet, int timeoutMS)
{
    if (m_bResend)
    {
        int count = RESEND_COUNT;
        while (!WaitPopLast(timeoutMS, packet))
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
                WARNING("resend addr:%d type:0x%02X fun:0x%02X", 
                        m_packet.GetDestAddr(),
                        m_packet.GetType(),
                        m_packet.GetFunction());
                DEBUG_HEX((char *)&m_packet, nSend);
            }
            else
            {
                ERROR("SendFrame write fail resend");
            }
        }
        return true;
    }
    else
    {
        return WaitPopLast(timeoutMS, packet);
    }
}

void CProtocolComm::EnableResend(bool bEnable)
{
    m_bResend = bEnable;
}

bool CProtocolComm::IsEnableResend()
{
    return m_bResend;
}
