#ifndef __REPEAT_COMM_H__
#define __REPEAT_COMM_H__
#include <deque>
#include "protocol_method.h"
#include "thread_wrap.h"

#ifdef _TEST
#include "led_gpio.h"
#endif

#define DATA_FILE "/etc/server485.dat"

class RepeatComm:public lib_linux::Thread
{
    public:
        enum
        {
            DATA_SETTING = 0,
            DATA_QUERY,
            DATA_HUKONG,
            DATA_OTHERS,

            // 灯光
            TYPE_LIGHT = 0,
            // 窗帘
            TYPE_CURTAIN,
            // 带新风功能的空调
            TYPE_AIRCONDITION,
            // 抄表接口
            TYPE_METER,

            // 人感
            TYPE_HUMAN_GROUP,
            // 漏水检查探头
            TYPE_LEAK_WATER,
            // 燃气检查探头
            TYPE_LEAK_GAS,
            TYPE_NULL,

            // 设备最多个数
            COUNT_MAX_LIGHT = 8,
            COUNT_MAX_CURTAIN = 8,
            COUNT_MAX_AIRCONDITION = 8,
            COUNT_MAX_METER = 4,
            COUNT_MAX_HUMAN_DEV = 34,
            COUNT_MAX_LEAK_DEV = 34,

            // 多个人感组成人感组，彼此或的关系
            COUNT_MAX_HUMAN_GROUP = 8,

            MAX_ROOM = 250,
            MAX_FLOOR = 220,
        };
        typedef BYTE DATA_RECORD[300];

        // 空调计量、电力、天然气、卫生热水、自来水查询数据保存
        typedef BYTE DATA_CALC[33];

        // 要保存的其他数据
        struct DATA_OTHERS_STRUCT
        {
            bool isInternet;
        };

    protected:
        // 定时查询线程
        class TimerQuery:public lib_linux::Thread
        {
            public:
                TimerQuery(RepeatComm &comm);
                ~TimerQuery();
            protected:
                void Run();
            private:
                RepeatComm &m_repeatcomm;
                // thread exit
                volatile bool m_bRunFlag;
#ifdef _TEST
                LedGPIO m_led;
#endif
        };

    public:
        RepeatComm();
        ~RepeatComm();

        // 打开2路485，启动转发线程
        bool OpenDevice485(const char *pstrSerial, int nBaudrate);
        bool OpenKongguanqi485(const char *pstrSerial, int nBaudrate);

        // 启动定时查询线程
        bool StartQueryThread();

        ProtocolMethod &GetMethod();

        ////////////////////////////////////////////////////////////
        // RPC 接口 外部线程调用
        //窗帘
        // @param
        //   inner_mode   ON/OFF/STOP/NONE  内窗帘
        //   outside_mode ON/OFF/STOP/NONE  外窗帘
        bool curtain(int nDestAddr, int inner_mode, int outside_mode);

        // 空调控制
        // switch_mode        ON/OFF
        // aircondition_mode  MODE_HOT/MODE_COLD
        // xinfeng_mode       1-纯新风 2-纯空调 3-新风+空调
        // bFixAutoXinfeng=false  新风自动逻辑必须设置人感为有人, 远大BUG提出的解决办法
        bool aircondition(int nDestAddr, int switch_mode, int aircondition_mode, int xinfeng_mode, int temp, int energy_save_mode, bool bFixAutoXinfengBug=false);

        // 查询空调状态
        bool query_aircondition(int nDestAddr, ProtocolMethod::aircondition_status &status, bool bSaveInBuffer=false);

        // 查找人感对应的设备
        struct DevInfo
        {
            BYTE type; // 设备类型
            BYTE addr; // 485地址
            BYTE opt; // 设备参数（可选）
        };
        int HumanSenToDev(BYTE btSenID, DevInfo *pInfo, int nLen);

        // 获得设备类型和个数
        struct DevCount
        {
            BYTE type; // 设备类型
            BYTE count; // 设备个数
        };
        int GetDeviceCount(DevCount *pDev, int nLen);

        // 获得楼层和地址码
        struct FloorRoom
        {
            int floor;
            int room;
        };
        FloorRoom GetFloorRoom();

        // 人感与灯控到绑定是否修改
        bool IsBindChange();

        // 设备数目是否变化
        bool IsDevChange();

        // 获得8路人感状态
        int GetSenStatus(BYTE *pData, int nLen);

        // 获得灯控节能模式的时间(10分钟或2小时), 返回秒
        int GetLightOffTime();

        ////////////////////////////////////////////////////////////////////////////////
        // 节能模式RPC
        //
        // 查询灯控节能模式(1-10分钟关, 2-2小时关)
        int QueryLightOffTime();
        // 设置灯控节能模式(1-10分钟关, 2-2小时关)
        bool SetLightOffTime(int index);

        // 获得隔热帘节能模式(1-1小时关闭, 2-4小时关闭, 3-8小时关闭, 4-人离不关)
        int QueryCurtainOffTime();
        // 设置隔热帘节能模式(1-1小时关闭, 2-4小时关闭, 3-8小时关闭, 4-人离不关)
        bool SetCurtainOffTime(int index);

        // 获得粉尘间隔时间(1-每天检测1次, 2-每天检测2次, 3-每天检测4次, 4-手动启动检测)
        int QueryDustTime();
        // 设置粉尘间隔时间(1-每天检测1次, 2-每天检测2次, 3-每天检测4次, 4-手动启动检测)
        bool SetDustTime(int index);

        // 获得新风运行定时参数
        struct XinfengTime
        {
            int nOFF; //人离阀关时间 单位秒
            int nON; //自动开阀时间 单位秒
        };
        XinfengTime GetXinfengTime();

        // 空气品质
        struct air_quality
        {
            int inner_PM03;
            int inner_PM25;
            int inner_PM10; 
            double inner_VOC;
            int inner_CO2;
            double inner_temp;
            int inner_humidity;
        };
        air_quality GetAirQuality();

        // 设置设备个数（调试用）
        void SetDeviceCount(int nLight, int nCurtain, int nMeter, int nXinfeng, bool bSave=true);

        // 空调计量
        void SetCalcAircondition(std::vector<double> &calc);

        // 电力、天然气查询
        void SetCalcPowerGas(std::vector<double> &calc);

        // 卫生热水、自来水查询
        void SetCalcWater(std::vector<double> &calc);

        // 设置价格
        void SetPrice(std::vector<double> &price);

        // 新风空调风速
        struct xinfeng_windspeed
        {
            double windspeed;
            double area;
        };
        xinfeng_windspeed GetXinfengWindSpeed();

        // 设置新风空管截面积
        void SetXinfengWindarea(double area);

        // 设置室外环境参数
        void SetOutsideEnv(std::vector<double> &data);

        // 设置室外传感器
        void SetOutsideSen(std::vector<int> &data);

        // 远程开启室内空气检测
        void SetInnerAirDetection();

        // 设置火警警报
        void SetFireAlarm(bool bAlarm);

        // 设置时间同步是否联网
        void SetTimeSynInternet(bool bInternet);
        bool GetTimeSynInternet();
        bool SetTime(int year, int month, int day, int hour, int minute, int second); 

        /////////////////////////////////////////////////////////
        // 报警数据
        // 查询室外传感器报警数据
        std::vector<int> QueryOutsideAlarm();
        // 查询室内传感器报警数据
        std::vector<int> QueryInnerAlarm();
        // 最近更新的警报信息
        std::vector<int> QueryInnerAlarmNew();

        // 新风时间控制
        struct XinfengTimeCtrl
        {
            int nLeave; //人离阀关时间 单位小时
            int nAutoRun; //自动开阀时间 单位分钟
        };
        XinfengTimeCtrl QueryXinfengTimeCtrl();
        bool SetXinfengTimeCtrl(XinfengTimeCtrl &time);

    protected:
        void Run();

        // 户控器操作
        void InnerProcess(Packet &packet);

        // 得到户控器上的4个开关量状态
        BYTE GetSwitchStatus();

        // 参数保存和读取
        void Save(int type, BYTE *pData, int nLen);
        void Load();

        // 传感器处理
        // 获得户控器和灯控器绑定的传感器类型
        int GetSenType(BYTE code);
        // 检查传感器绑定的是否有效
        bool IsSenHumanType(BYTE code);
        // 处理触发的开关量，对照传感器关联
        void ProcessSen();

        // 人感关联新风接口，查询是否有人
        bool IsHasManByXinfeng(BYTE addr);

        // 人感关联窗帘，查询是否有人
        bool IsHasManByCurtain(BYTE addr);

        // 获得设备类型和个数(内部用)
        int RawGetDeviceCount(DevCount *pDev, int nLen);

        // 人感状态辅助函数
        // 开关量转是否触发
        bool TranSenActive(int status);
        // 是否触发转开关量
        int TranSwitchStatus(bool bActive);
        // gpio值转开关量
        int TranGPIOToSwitchStatus(int gpio);

        // 抄表时间是否同步
        bool IsMeterSyncTime();

        // 查询窗帘是否有阳光
        bool IsCurtainHasSunshine(BYTE btAddr);

    private:
        // control thread exit
        volatile bool m_bRunFlag;

        // 设置参数
        DATA_RECORD m_data_setting;
        // 工况查询
        DATA_RECORD m_data_query_setting;
        // 户控接口查询
        DATA_RECORD m_data_hukong;
        // 新风接口参数
        DATA_RECORD m_data_xinfeng;


        // 保存户控器和灯控器开关量状态
        // 0-断开 1-闭合
        // 0 - 户控器
        // 1 - 8 灯控器
        DATA_RECORD m_data_switch_status;

        // 传感器状态  1-触发 0-没触发
        // 0 - 无效
        // 1 - 8 人感
        // 9 - 12 漏水探测
        // 13 - 燃气泄露
        DATA_RECORD m_data_sen_status;

        // 空调计量
        DATA_CALC m_data_calc_aircondition;

        // 电力、天然气查询
        DATA_CALC m_data_calc_power_gas;

        // 卫生热水、自来水查询
        DATA_CALC m_data_calc_water;

        // 室外环境参数
        BYTE m_data_outside_env[16];

        // 室外传感器参数
        BYTE m_data_outside_sen[16];
        // 室外温度
        double m_outside_temp;
        // 季节
        BYTE m_btSeason;
        // 东面阳光信号
        bool m_bEastSun;
        // 南面阳光信号
        bool m_bSouthSun;
        // 西面阳光信号
        bool m_bWestSun;

        // 远程开启室内空气检测
        bool m_bInnerAirDetection;

        // 是否有警报
        volatile bool m_bFireAlarm;
        // 楼控中心下发的火警警报
        volatile bool m_bBroadcastFireAlarm;

        // 其他要保存的数据（如是否联网)
        DATA_OTHERS_STRUCT m_data_others;

        // 控管器是否修改传感器绑定
        volatile bool m_isBindChange;

        // 是否设备数目变化
        volatile bool m_isDevChange;

        // 是否同步抄表时间
        volatile bool m_isMeterSyncTime;

        // 新风模式和温度同步空管器，特殊处理
        volatile int m_sync_xinfeng_temp;
        volatile int m_sync_xinfeng_mode;

        // 新风控制器BUG特殊处理
        std::map<int, int> m_mapFixAutoXinfengBug;
        std::map<int, int> m_mapFixAutoXinfengBugCount;

        // 保护锁
        lib_linux::Mutex m_mutex;

    private:
        ProtocolMethod m_method;
        CProtocolComm  m_comm;
        TimerQuery     m_timerQuery;

        // 空管器是否下发了设置命令，要优先处理
        volatile bool m_isKongguanSetCmd;
};

#endif
