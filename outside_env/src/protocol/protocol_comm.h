//////////////////////////////////////////////////////////////////////////////
//                          利尔达科技有限公司
//说    明: 协议解析类
//作    者: 钟柳波
//最后修改: 
//          2010-7-7：增加了Clear()函数用于清除数据帧Buffer，主要是防止在接收数据帧时有多余的数据从而
//                    导致数据帧错误的Bug。   
//          2010-7-12: 修改了SendFrame函数参数，移植新协议
//          2011-6-2: 文件剥离Dll,并入主程序。
//          2011-6-24: 支持串口和网络
//////////////////////////////////////////////////////////////////////////////
#ifndef   _PROTOCOL_COMM_H_
#define   _PROTOCOL_COMM_H_

#include "protocol_packet.h"
#include "serialport.h"
#include "jobqueue.h" // from lib-linux
#include <deque>

/////////////////////////////////////////////////////////////////////////
//串口协议处理类
class CProtocolComm:public Serialport
{
    public:
        enum
        {
            // modbus没有重发
            RESEND_COUNT = 1,
        };

        // 室外环境参数
        struct env_info
        {
            unsigned int PM03;
            unsigned int PM25;
            unsigned int PM10;
            double jiaquang;
            unsigned int CO2;
            double temp;
            unsigned int wet;

            // 0夏季 1冬季
            BYTE season;
            bool bEastsun;
            bool bSouthsun;
            bool bWetsun;
            bool bSunsen1Fault;
            bool bSunsen2Fault;
            bool bSunsen3Fault;
            bool bDustsenFault;
            bool bWetsenFault;
            bool bTempsenFault;
        };

        // 设置室外环境参数
        struct setting_info 
        {
            int SummerDays;
            int SummerTemp;
            int SummerTempContinue;

            int WinterDays;
            int WinterTemp;
            int WinterTempContinue;

            int LightStrength;
            bool OutsideAirDetection;
        };

    public:
        CProtocolComm();
        ~CProtocolComm();

        // 工况查询
        bool QueryEnv(env_info &info);

        // 设置
        bool SetEnv(setting_info &info);
    protected:
        //发送数据包
        bool SendFrame(BYTE btAddr, BYTE btCmd, WORD wRegAddr, WORD wRegAddrCount, void *pData, BYTE btLen);

        //接收应答数据包
        // timeoutMS 超时(毫秒)
        // 返回是否超时
        bool WaitResponse(Packet &packet, int timeoutMS=TIMEOUT);

        //接收数据
        void OnReceive(int nLen);
    private:
        // lock to protect 485 SendFrame
        lib_linux::Mutex m_mutex;

        //serialport缓冲区,必须在线程中使用
        PacketBuffer m_buffer;

        //支持等待的线程队列
        lib_linux::JobQueue<Packet> m_queue;

        //发送数据包
        Packet m_packet;
};

#endif 
