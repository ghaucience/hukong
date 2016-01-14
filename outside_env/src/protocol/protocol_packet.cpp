//
//  @author   lbzhung
//  @brief    packet define
//  @date     2014.2.13
//
#include <numeric>
#include <assert.h>
#include <asm/byteorder.h>
#include "crc16.h"

#include "protocol_packet.h"
#include "lib_linux_config.h"

/////////////////////////////////////////////////////////////////////////////
//  packet
WORD Packet::CheckSUM(BYTE *pData, int nLen)
{
    // modbus crc is oxFFFF, don't ask me why
    return ::crc16(0xFFFF, pData, nLen);
}

int Packet::BuildPacket(BYTE btAddr, BYTE btCmd, WORD wRegAddr, WORD wRegAddrCount, void *pData, BYTE btLen)
{
    m_btAddr = btAddr;
    m_btCmd = btCmd;
    *(WORD *)&m_Data[0] = __cpu_to_be16(wRegAddr);
    *(WORD *)&m_Data[2] = __cpu_to_be16(wRegAddrCount);
    int nPacketLen = 6;                                                      

    // copy data
    if (pData != NULL)
    {
        m_Data[4] = btLen;
        memcpy(&m_Data[5], pData, btLen);  
        nPacketLen += (btLen+1);
    }

    // checksum
    *(WORD *)(&m_btAddr+nPacketLen) = __cpu_to_le16(CheckSUM(&m_btAddr, nPacketLen));
    nPacketLen += 2;

    // return all packet length
    return nPacketLen;
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
        // check length
        if (size() >= Packet::PACKET_MINLEN &&
            size() >= pPacket->GetPacketLen())
        {
            // check checksum and packet end
            WORD wCRC = __le16_to_cpu(*(WORD *)&(pPacket->GetData()[pPacket->GetDataLen()]));
            WORD wCalCRC = pPacket->CheckSUM(&pPacket->m_btAddr, pPacket->GetPacketLen()-2);
            if (wCRC == wCalCRC)
            {
                DEBUG("recv: %d", size());
                DEBUG_HEX(raw(), size());
                return true;
            }
            ERROR("serialport packet checksum wrong calc:%04x recv:%04X", wCalCRC, wCRC);
            DEBUG_HEX(raw(), size());
        }
        else
        {
            return false;
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
