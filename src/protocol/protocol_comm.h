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

#include <string>
#include <deque>
#include <map>
#include <set>
#include "protocol_packet.h"
#include "serialport.h"
#include "Mutex.h" // from lib-linux
#include "jobqueue.h" // from lib-linux
#include "led_gpio.h"

/////////////////////////////////////////////////////////////////////////
//串口协议处理类
class CProtocolComm:public Serialport
{
public:
    enum
    {
        RESEND_COUNT = 3,
    };

public:
	CProtocolComm();

    ~CProtocolComm();

	//发送数据包
	bool SendFrame(BYTE btSrcAddr, BYTE btDestAddr,
                   BYTE btType, BYTE btFunction, void *pData, BYTE btLen, bool bClearRecvQueue=true);

	//发送数据包
	bool SendFrame(Packet &packet);

    // 发送应答数据包（需要特殊处理)
    // remain_packet 是当前接受队列收到的命令数据包
    bool SendResponseFrame(DWORD dwRequestPacketID, Packet &packet);

    //接收应答数据包
    // timeoutMS 超时(毫秒)
    // bStopSReSendTemp  暂时停止重发
    // 返回是否超时
    bool WaitResponse(Packet &packet, int timeoutMS=TIMEOUT);

    //开启和关闭重发机制
    void EnableResend(bool bEnable);
    bool IsEnableResend();
protected:
    // 等待数据包，返回最后一包数据
    bool WaitPopLast(int nTimeoutMS, Packet &packet);

	//接收数据
	void OnReceive(int nLen);
private:
    //serialport缓冲区,必须在线程中使用
	PacketBuffer m_buffer;

    //支持等待的线程队列
    lib_linux::JobQueue<Packet> m_queue;

    //是否支持重发机制
    volatile bool m_bResend;

    //发送数据包
    Packet m_packet;

    // 485冲突led gpio
    LedGPIO m_led;
};

#endif 
