//
//  @author   lbzhung
//  @brief    packet define
//  @date     2013.4.26
//  @modify   2013.5.2 Fix PACKET_MINLEN check
//
#include <numeric>
#include <assert.h>
#include <asm/byteorder.h>

#include "protocol_packet.h"
#include "lib_linux_config.h"

/////////////////////////////////////////////////////////////////////////////
//  packet
BYTE Packet::CheckSUM(BYTE *pData, int nLen)
{
    return (BYTE)(std::accumulate(pData, pData + nLen, 0));
}

int Packet::BuildPacket(BYTE btSrcAddr, BYTE btDestAddr,
                        BYTE btType, BYTE btFunction, void *pData, BYTE btLen)
{
    m_wHead = __be16_to_cpu(PACKET_HEAD);                                            
    m_btSrcAddr = btSrcAddr;
    m_btDestAddr = btDestAddr;
    m_btType = btType;
    m_btFunction = btFunction;

    // origion length
    m_btLen = 5;                                                      

    // copy data
    if (pData != NULL)
    {
        memcpy(m_Data, pData, btLen);  
        m_btLen += btLen;
    }

    // checksum
    m_Data[GetDataLen()] = CalcCheckSUM();
    m_Data[GetDataLen()+1] = PACKET_END;

    // return all packet length
    return GetPacketLen();
}

////////////////////////////////////////////////////////////////////////
//
// packet buffer
//
PacketBuffer::PacketBuffer()
{
}

bool PacketBuffer::PacketVaild()
{
    if (size() > 0)
    {
        //DEBUG("recv: %d", size());
        //DEBUG_HEX(raw(), size());
        Packet *pPacket = (Packet *)raw();

        if (size() < (int)(sizeof(pPacket->m_wHead)))
        {
            return false;
        }

        // check head
        if (pPacket->m_wHead == __be16_to_cpu(Packet::PACKET_HEAD))
        {
            // check length
            if (size() >= Packet::PACKET_MINLEN &&
                size() >= pPacket->GetPacketLen())

            {
                // check checksum and packet end
                if (pPacket->IsCheckSumOk() &&
                    pPacket->m_Data[pPacket->GetDataLen()+1] == Packet::PACKET_END)
                {
                    DEBUG("recv: %d", size());
                    DEBUG_HEX(raw(), size());
                    return true;
                }
                ERROR("serialport packet checksum wrong");
                DEBUG_HEX(raw(), size());
            }
            else
            {
                return false;
            }
        }
        else
        {
            ERROR("serialport packet head is wrong in PacketVaild");
            DEBUG_HEX(raw(), size());
        }

        // throw exception
        throw(Packet::EXCEPTION_PACKET);
    }
    else
    {
        return false;
    }
}

bool PacketBuffer::PreProcess()
{
    return false;
}

bool PacketBuffer::GetPacket(Packet &packet)
{
    // check packet
    if (PacketVaild()) 
    {
        Packet *pPacket = (Packet *)raw();
        //INFO("Has a Packet");
        // block unnecessary packet
        if (PreProcess())
        {
            // delete a packet
            del(pPacket->GetPacketLen());
            return false;
        }
        else
        {
            // copy packet
            //packet = *pPacket; // this may dangerous
            ::memcpy(&packet, pPacket, pPacket->GetPacketLen());

            del(pPacket->GetPacketLen());
            return true;
        }
    }
    return false;
}
