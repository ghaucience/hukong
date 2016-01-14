//
//  @author   lbzhung
//  @brief    packet define
//  @date     2014.2.13
//
#ifndef  _PROTOCOL_PACKET_H_
#define  _PROTOCOL_PACKET_H_

#include "buffer.h"
#include "protocol_config.h"

// 参考室外环境检测板技术资料

//////////////////////////////////////////////////////////////////////////
//
//  packet def
//
#pragma pack(1)
struct Packet
{
    public:
        // 注意！！！接收数据包才能识别包长度
        inline int GetPacketLen()
        {
            // 现阶段只支持工况查询和设置
            assert(m_btCmd == 0x03 || m_btCmd == 0x10);
            switch (m_btCmd)
            {
                case 0x10:
                    return 8;
                case 0x03:
                default:
                    return m_Data[0] + 5;
            }
        }

        // 注意！！！接收数据包才能识别数据长度
        inline int GetDataLen(void)
        {
            // 现阶段只支持工况查询和设置
            assert(m_btCmd == 0x03 || m_btCmd == 0x10);
            switch (m_btCmd)
            {
                case 0x10:
                    return 0;
                case 0x03:
                default:
                    return m_Data[0];
            }
        }

        // 注意！！！接收数据包才能取出数据
        inline BYTE *GetData()
        {
            // 现阶段只支持工况查询和设置
            assert(m_btCmd == 0x03 || m_btCmd == 0x10);
            switch (m_btCmd)
            {
                case 0x10:
                    return &m_Data[4];
                case 0x03:
                default:
                    return &m_Data[1];
            }
        }

        inline BYTE GetCmd()
        {
            return m_btCmd;
        }

        inline void SetCmd(BYTE btCmd)
        {
            m_btCmd = btCmd;
        }

        inline bool IsReturnOK()
        {
            //return m_Data[0] == PACKET_CMD_OK;
            return false;
        }

        // build packet
        int BuildPacket(BYTE btAddr, BYTE btCmd, WORD wRegAddr, WORD wRegAddrCount, void *pData, BYTE btLen);

    protected:
        //计算checksum CRC
        static WORD CheckSUM(BYTE *pData, int nLen);

    private:
        enum{
            // min packet length
            PACKET_MINLEN = 6, 

            // return ok flag
            //PACKET_CMD_OK = 0x00, 
            // exception 
            EXCEPTION_PACKET = 1,
        };

        BYTE m_btAddr;       // head
        BYTE m_btCmd;       // cmd code
        BYTE m_Data[256];
        //WORD wChecksum;  // checksum

        friend class PacketBuffer;
};
#pragma pack()

////////////////////////////////////////////////////////////
// 
//  packet buffer
//
class PacketBuffer:public lib_linux::Buffer
{
    public:
        PacketBuffer();

        // get packet from buffer after parse,
        // return true if has a packet.
        bool GetPacket(Packet &packet);

    protected:
        // check packet vaild
        bool PacketVaild();

        // process before get packet
        bool PreProcess();
};

#endif
