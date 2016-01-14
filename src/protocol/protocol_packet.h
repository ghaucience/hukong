//
//  @author   lbzhung
//  @brief    packet define
//  @date     2013.4.26
//
#ifndef  _PROTOCOL_PACKET_H_
#define  _PROTOCOL_PACKET_H_

#include "buffer.h"
#include "protocol_config.h"

// Send and Receive
//                 ----------------------------length-------------------------------
//                 |                                                                |
//  ---------------------------------------------------------------------------------------
// | Head | Length | SrcAddr | DestAddr | TypeCode | FunCode | Data block | Checksum | End |
// ----------------------------------------------------------------------------------------
// 2(0xF7F8)  1        1         1           1         1         0~N          1      1(0xFD)
//        ^                                                               |
//        -----------------------------checksum----------------------------
//

//////////////////////////////////////////////////////////////////////////
//
//  packet def
//
#pragma pack(1)
struct Packet
{
    public:
        inline int GetPacketLen()
        {
            return m_btLen + 4;
        }

        inline int GetDataLen(void)
        {
            return m_btLen - 5;
        }

        inline BYTE *GetData()
        {
            return m_Data;
        }

        inline BYTE GetSrcAddr()
        {
            return m_btSrcAddr;
        }

        inline BYTE GetDestAddr()
        {
            return m_btDestAddr;
        }

        inline BYTE GetType()
        {
            return m_btType;
        }

        inline void SetType(BYTE btType)
        {
            m_btType = btType;
        }

        inline BYTE GetFunction()
        {
            return m_btFunction;
        }

        inline void SetFunction(BYTE btFun)
        {
            m_btFunction = btFun;
        }

        inline bool IsReturnOK()
        {
            //return m_Data[0] == PACKET_CMD_OK;
            return true;
        }

        inline void ReCheckSUM()
        {
            m_Data[GetDataLen()] = CalcCheckSUM();
        }

        // 用于识别数据包的类型
        inline DWORD GetPacketTypeID()
        {
            return GetPacketTypeID(m_btSrcAddr, m_btDestAddr, m_btType, m_btFunction);
        }

        // 用于识别数据包
        inline DWORD GetPacketID()
        {
            // 用校验码识别
            return m_Data[GetDataLen()];
        }

        static DWORD GetPacketTypeID(BYTE btSrcAddr, BYTE btDestAddr, BYTE btType, BYTE btFunction)
        {
            return btSrcAddr<<24 | \
                   btDestAddr<<16 | \
                   btType<<8 | \
                   btFunction;
        }

        // build packet
        int BuildPacket(BYTE btSrcAddr, BYTE btDestAddr,
                        BYTE btType, BYTE btFunction, void *pData, BYTE btLen);

    protected:
        //计算checksum
        static BYTE CheckSUM(BYTE *pData, int nLen);

        BYTE CalcCheckSUM()
        {
            return CheckSUM(&m_btLen, GetPacketLen()-4);
        }

        bool IsCheckSumOk()
        {
            return CalcCheckSUM() == m_Data[GetDataLen()];
        }

    private:
        enum{
            // min packet length
            PACKET_MINLEN = 9, 

            // head
            PACKET_HEAD = 0xF7F8, 
            PACKET_END = 0xFD,

            // return ok flag
            //PACKET_CMD_OK = 0x00, 
            // exception 
            EXCEPTION_PACKET = 1,
        };

        WORD m_wHead;       // head
        BYTE m_btLen;       // length
        BYTE m_btSrcAddr;   // source addr
        BYTE m_btDestAddr;  // destination addr
        BYTE m_btType;      // type code
        BYTE m_btFunction;  // function code
        BYTE m_Data[255];   // data block
        //BYTE btChecksum;  // checksum
        //BYTE btEnd;       // end

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
