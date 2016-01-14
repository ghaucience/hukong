#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include "rpc_config.h"
#include "protocol_method.h"
#include "repeat_comm.h"

#ifdef _TEST
#define RPC_LOG INFO
#else
#define RPC_LOG DEBUG
#endif

// check call fail
#define CHECK_METHOD_CALL(x) \
    if (!x)\
    {\
        throw(xmlrpc_c::fault("maybe serialport call cmd timeout", (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    } 

#define METHOD_DEF(x) class C##x:public xmlrpc_c::method {\
    public:\
           C##x(RepeatComm &repeatComm)\
           :m_repeatComm(repeatComm)\
           {}\
           void execute(xmlrpc_c::paramList const& paramList, xmlrpc_c::value *const  retvalP)
#define METHOD_END  private:\
                        RepeatComm &m_repeatComm;\
                    };

METHOD_DEF(light_switch)
{
    int nDestAddr = paramList.getInt(0);
    int nIndex =  paramList.getInt(1);
    int mode = paramList.getInt(2);
    paramList.verifyEnd(3);

    RPC_LOG("light_switch addr:%d index:%d mode:%d", nDestAddr, nIndex, mode);
    try{
        bool bRet = m_repeatComm.GetMethod().light_switch(nDestAddr, nIndex, mode);
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }

    //*retvalP = xmlrpc_c::value_nil();
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(query_light_switch)
{
    int nDestAddr = paramList.getInt(0);
    paramList.verifyEnd(1);

    RPC_LOG("query_light_switch %d", nDestAddr);
    ProtocolMethod::light_switch_status status;
    try
    {
        bool bRet = m_repeatComm.GetMethod().query_light_switch(nDestAddr, status);
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }

    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["light1_status"]           = xmlrpc_c::value_int(status.light1_status);
    rpc_struct["light2_status"]           = xmlrpc_c::value_int(status.light2_status);
    rpc_struct["light3_status"]           = xmlrpc_c::value_int(status.light3_status);
    rpc_struct["light4_status"]           = xmlrpc_c::value_int(status.light4_status);
    rpc_struct["light5_status"]           = xmlrpc_c::value_int(status.light5_status);
    rpc_struct["light6_status"]           = xmlrpc_c::value_int(status.light6_status);
    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(inner_curtain)
{
    int nDestAddr = paramList.getInt(0);
    int mode = paramList.getInt(1);
    paramList.verifyEnd(2);

    RPC_LOG("inner_curtain addr:%d mode:%d", nDestAddr, mode);

    try
    {
        bool bRet = m_repeatComm.curtain(nDestAddr, mode, ProtocolMethod::NONE);
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }

    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(outside_curtain)
{
    int nDestAddr = paramList.getInt(0);
    int mode = paramList.getInt(1);
    paramList.verifyEnd(2);

    RPC_LOG("outside_curtain addr:%d mode:%d", nDestAddr, mode);

    try
    {
        bool bRet = m_repeatComm.curtain(nDestAddr, ProtocolMethod::NONE, mode);
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(query_curtain)
{
    int nDestAddr = paramList.getInt(0);
    paramList.verifyEnd(1);

    RPC_LOG("query curtain addr:%d ", nDestAddr);

    ProtocolMethod::curtain_status status;
    try
    {
        bool bRet = m_repeatComm.GetMethod().query_curtain(nDestAddr, status);
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }

    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["inner"]           = xmlrpc_c::value_int(status.inner);
    rpc_struct["outside"]           = xmlrpc_c::value_int(status.outside);
    rpc_struct["inner_fault"]           = xmlrpc_c::value_int(status.inner_fault);
    rpc_struct["outside_fault"]           = xmlrpc_c::value_int(status.outside_fault);
    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(query_curtain_verbose)
{
    int nDestAddr = paramList.getInt(0);
    paramList.verifyEnd(1);

    RPC_LOG("query curtain verbose addr:%d ", nDestAddr);
    ProtocolMethod::curtain_status status;
    try
    {
        bool bRet = m_repeatComm.GetMethod().query_curtain(nDestAddr, status);
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }

    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["inner"]           = xmlrpc_c::value_int(status.inner);
    rpc_struct["outside"]           = xmlrpc_c::value_int(status.outside);
    rpc_struct["has_man"]           = xmlrpc_c::value_int(status.has_man);
    rpc_struct["control"]           = xmlrpc_c::value_int(status.control);
    rpc_struct["season"]           = xmlrpc_c::value_int(status.season);
    rpc_struct["sunshine"]           = xmlrpc_c::value_int(status.sunshine);
    rpc_struct["outside_temp"]           = xmlrpc_c::value_double(status.outside_temp);
    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(query_meter)
{
    int nDestAddr = paramList.getInt(0);
    paramList.verifyEnd(1);

    RPC_LOG("query meter addr:%d ", nDestAddr);
    ProtocolMethod::meter_status status;
    try
    {
        bool bRet = m_repeatComm.GetMethod().query_meter(nDestAddr, status);
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }

    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["cold_cur_rate"] = xmlrpc_c::value_double(status.cold_cur_rate);
    rpc_struct["hot_cur_rate"] =  xmlrpc_c::value_double(status.hot_cur_rate);
    rpc_struct["cold_totoal"] =  xmlrpc_c::value_double(status.cold_totoal);
    rpc_struct["hot_totoal"] =   xmlrpc_c::value_double(status.hot_totoal);
    rpc_struct["hot_input_temp"] =   xmlrpc_c::value_double(status.hot_input_temp);
    rpc_struct["hot_output_temp"] =  xmlrpc_c::value_double(status.hot_output_temp);
    rpc_struct["hot_totoal_degree"] = xmlrpc_c::value_double(status.hot_totoal_degree);
    rpc_struct["hot_cur_power"] =  xmlrpc_c::value_double(status.hot_cur_power);
    rpc_struct["cur_totoal_power"] = xmlrpc_c::value_double(status.cur_totoal_power);
    rpc_struct["forward_totoal_degree"] = xmlrpc_c::value_double(status.forward_totoal_degree);
    rpc_struct["gas_cur_rate"] = xmlrpc_c::value_double(status.gas_cur_rate);
    rpc_struct["gas_totoal_rate"] = xmlrpc_c::value_double(status.gas_totoal_rate);
    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(aircondition)
{
    int nDestAddr = paramList.getInt(0);
    int switch_mode = paramList.getInt(1);
    int aircondition_mode = paramList.getInt(2);
    int xinfeng_mode = paramList.getInt(3);
    int temp = paramList.getInt(4);
    int energy_save_mode = paramList.getInt(5);
    paramList.verifyEnd(6);

    RPC_LOG("aircondition addr:%d switch_mode:%d aircondition_mode:%d xinfeng_mode:%d temp:%d, energy_save_mode:%d",
          nDestAddr, switch_mode, aircondition_mode, xinfeng_mode, temp, energy_save_mode);
    try
    {
        bool bRet = m_repeatComm.aircondition(nDestAddr, switch_mode, aircondition_mode, xinfeng_mode, temp, energy_save_mode);
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        ERROR("aircondition error:%s ", pstrError);
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(aircondition_svm)
{
    int nDestAddr = paramList.getInt(0);
    int switch_mode = paramList.getInt(1);
    int aircondition_mode = paramList.getInt(2);
    int xinfeng_mode = paramList.getInt(3);
    int temp = paramList.getInt(4);
    int energy_save_mode = paramList.getInt(5);
    paramList.verifyEnd(6);

    RPC_LOG("aircondition_svm addr:%d switch_mode:%d aircondition_mode:%d xinfeng_mode:%d temp:%d, energy_save_mode:%d",
          nDestAddr, switch_mode, aircondition_mode, xinfeng_mode, temp, energy_save_mode);
    try
    {
        bool bRet = m_repeatComm.aircondition(nDestAddr, switch_mode, aircondition_mode, xinfeng_mode, temp, energy_save_mode, true);
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        ERROR("aircondition_svm error:%s ", pstrError);
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(query_aircondition)
{
    int nDestAddr = paramList.getInt(0);
    paramList.verifyEnd(1);

    ProtocolMethod::aircondition_status status;
    try
    {
        bool bRet = m_repeatComm.GetMethod().query_aircondition(nDestAddr, status, false, 25);
        if (bRet)
        {
            RPC_LOG("query_aircondition addr:%d status:%d temp:%d", nDestAddr, status.switch_mode, status.temp_setting);
        }
        else
        {
            RPC_LOG("query_aircondition addr:%d error", nDestAddr);
        }
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        ERROR("query_aircondition error:%s ", pstrError);
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }

    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["switch_mode"] = xmlrpc_c::value_int(status.switch_mode);
    rpc_struct["aircondition_mode"] =  xmlrpc_c::value_int(status.aircondition_mode);
    rpc_struct["xinfeng_mode"] =  xmlrpc_c::value_int(status.xinfeng_mode);
    rpc_struct["temp_setting"] =  xmlrpc_c::value_int(status.temp_setting);
    rpc_struct["energy_save_mode"] = xmlrpc_c::value_int(status.energy_save_mode);
    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(query_aircondition_verbose)
{
    int nDestAddr = paramList.getInt(0);
    paramList.verifyEnd(1);

    ProtocolMethod::aircondition_status status;
    try
    {
        bool bRet = m_repeatComm.GetMethod().query_aircondition(nDestAddr, status, false, 25);
        if (bRet)
        {
            RPC_LOG("query_aircondition_verbose addr:%d status:%d temp:%d", nDestAddr, status.switch_mode, status.temp_setting);
        }
        else
        {
            RPC_LOG("query_aircondition_verbose addr:%d error", nDestAddr);
        }
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        ERROR("query_aircondition_verbose error:%s ", pstrError);
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }

    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["switch_mode"] = xmlrpc_c::value_int(status.switch_mode);
    rpc_struct["aircondition_mode"] =  xmlrpc_c::value_int(status.aircondition_mode);
    rpc_struct["xinfeng_mode"] =  xmlrpc_c::value_int(status.xinfeng_mode);
    rpc_struct["temp_setting"] =  xmlrpc_c::value_int(status.temp_setting);
    rpc_struct["energy_save_mode"] = xmlrpc_c::value_int(status.energy_save_mode);

    rpc_struct["xinfeng_alarm_1"] = xmlrpc_c::value_int(int(status.bFireAlarm));
    rpc_struct["xinfeng_alarm_2"] = xmlrpc_c::value_int(int(GET_BIT(status.btAlarm1, 0)));
    rpc_struct["xinfeng_alarm_3"] = xmlrpc_c::value_int(int(GET_BIT(status.btAlarm1, 1)));
    rpc_struct["xinfeng_alarm_4"] = xmlrpc_c::value_int(int(GET_BIT(status.btAlarm1, 2)));
    rpc_struct["xinfeng_alarm_5"] = xmlrpc_c::value_int(int(GET_BIT(status.btAlarm1, 3)));
    rpc_struct["xinfeng_alarm_6"] = xmlrpc_c::value_int(int(GET_BIT(status.btAlarm1, 4)));
    rpc_struct["xinfeng_alarm_7"] = xmlrpc_c::value_int(int(GET_BIT(status.btAlarm1, 5)));
    rpc_struct["xinfeng_alarm_8"] = xmlrpc_c::value_int(int(GET_BIT(status.btAlarm1, 6)));
    rpc_struct["xinfeng_alarm_9"] = xmlrpc_c::value_int(int(GET_BIT(status.btAlarm1, 7)));
    rpc_struct["xinfeng_alarm_10"] = xmlrpc_c::value_int(int(GET_BIT(status.btAlarm2, 0)));

    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(aircondition_set_mode)
{
    int nDestAddr = paramList.getInt(0);
    int aircondition_mode = paramList.getInt(1);
    paramList.verifyEnd(2);

    RPC_LOG("aircondition_set_mode addr:%d mode:%d", nDestAddr, aircondition_mode);
    try
    {
        ProtocolMethod::aircondition_status status;
        bool bRet = m_repeatComm.GetMethod().query_aircondition(nDestAddr, status, false, 25);
        CHECK_METHOD_CALL(bRet)

        bRet = m_repeatComm.aircondition(nDestAddr, status.switch_mode, aircondition_mode, status.xinfeng_mode, status.temp_setting, status.energy_save_mode);
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        ERROR("aircondition_set_mode error:%s ", pstrError);
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

////////////////////////////////////////////////////////////////////////////
// 户控器sedona调用
METHOD_DEF(humansen_to_dev)
{
    RepeatComm::DevInfo infos[256];
    ::memset(infos, 0, sizeof(infos));
    int nSenID = paramList.getInt(0);
    paramList.verifyEnd(1);

    int nRet = m_repeatComm.HumanSenToDev(nSenID, infos, sizeof(infos)/sizeof(infos[0]));

    xmlrpc_c::carray rpc_array;
    for (int i=0; i<nRet; i++)
    {
        // 把结构体编码成数字
        unsigned int code = (unsigned int)(infos[i].type)<<16 | \
                            (unsigned int)(infos[i].addr)<<8 | \
                            (unsigned int)(infos[i].opt);
        rpc_array.push_back(xmlrpc_c::value_int(code));
    }
    *retvalP = xmlrpc_c::value_array(rpc_array);
}
METHOD_END

METHOD_DEF(is_bind_change)
{
    RPC_LOG("is_bind_change");
    bool bChange = m_repeatComm.IsBindChange();
    *retvalP = xmlrpc_c::value_boolean(bChange);
}
METHOD_END

METHOD_DEF(get_sen_status)
{
    BYTE data[RepeatComm::COUNT_MAX_HUMAN_GROUP];
    RPC_LOG("get_sen_status");
    int nRet = m_repeatComm.GetSenStatus(data, sizeof(data));
    xmlrpc_c::carray rpc_array;
    for (int i=0; i<nRet; i++)
    {
        rpc_array.push_back(xmlrpc_c::value_int(data[i]));
    }
    *retvalP = xmlrpc_c::value_array(rpc_array);
}
METHOD_END

METHOD_DEF(get_light_off_time)
{
    RPC_LOG("get_light_off_time");
    int nTime = m_repeatComm.GetLightOffTime(); 
    *retvalP = xmlrpc_c::value_int(nTime);
}
METHOD_END

METHOD_DEF(get_device_count)
{
    RPC_LOG("get_device_count");
    RepeatComm::DevCount dev[10];
    int nRet = m_repeatComm.GetDeviceCount(dev, sizeof(dev)/sizeof(dev[0]));
    xmlrpc_c::carray rpc_array;
    for (int i=0; i<nRet; i++)
    {
        xmlrpc_c::cstruct rpc_struct;
        rpc_struct["type"] = xmlrpc_c::value_int(dev[i].type);
        rpc_struct["count"] =  xmlrpc_c::value_int(dev[i].count);
        rpc_array.push_back(xmlrpc_c::value_struct(rpc_struct));
    }
    *retvalP = xmlrpc_c::value_array(rpc_array);
}
METHOD_END

METHOD_DEF(get_floor_room)
{
    RPC_LOG("get_floor_room");
    RepeatComm::FloorRoom fa = m_repeatComm.GetFloorRoom();
    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["floor"] = xmlrpc_c::value_int(fa.floor);
    rpc_struct["room"] =  xmlrpc_c::value_int(fa.room);
    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(get_air_quality)
{
    RPC_LOG("get_air_quality");
    RepeatComm::air_quality air = m_repeatComm.GetAirQuality();
    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["inner_PM03"] = xmlrpc_c::value_int(air.inner_PM03);
    rpc_struct["inner_PM25"] =  xmlrpc_c::value_int(air.inner_PM25);
    rpc_struct["inner_PM10"] =  xmlrpc_c::value_int(air.inner_PM10);
    rpc_struct["inner_VOC"] =  xmlrpc_c::value_double(air.inner_VOC);
    rpc_struct["inner_CO2"] =  xmlrpc_c::value_int(air.inner_CO2);
    rpc_struct["inner_temp"] =  xmlrpc_c::value_double(air.inner_temp);
    rpc_struct["inner_humidity"] =  xmlrpc_c::value_int(air.inner_humidity);
    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(get_xinfeng_time)
{
    RepeatComm::XinfengTime status = m_repeatComm.GetXinfengTime();
    RPC_LOG("get_xinfeng_time:ON %d OFF %d", status.nON, status.nOFF);
    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["nOFF"] =  xmlrpc_c::value_int(status.nOFF);
    rpc_struct["nON"] = xmlrpc_c::value_int(status.nON);
    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(get_xinfeng_switch_status)
{
    int nDestAddr = paramList.getInt(0);
    paramList.verifyEnd(1);

    RPC_LOG("get_xinfeng_switch_status");
    ProtocolMethod::aircondition_status status;
    try
    {
        // 直接查询新风缓存数据
        bool bRet = m_repeatComm.GetMethod().query_aircondition(nDestAddr, status, false, 25);
        CHECK_METHOD_CALL(bRet)
    }
    catch(const char *pstrError)
    {
        throw(xmlrpc_c::fault(pstrError, (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));\
    }

    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["nXinfengSwitch"] =  xmlrpc_c::value_int(status.nXinfengSwitch);
    rpc_struct["nTiaofengSwitch"] = xmlrpc_c::value_int(status.nTiaofengSwitch);
    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(set_calc_aircondition)
{
    std::vector<xmlrpc_c::value> value = paramList.getArray(0, 12, 12);
    paramList.verifyEnd(1);

    std::vector<double> calc;
    for (std::vector<xmlrpc_c::value>::iterator it=value.begin();
                                                it!=value.end();
                                                it++)
    {
        double const doublevalue(static_cast<double>(xmlrpc_c::value_double(*it)));
        calc.push_back(doublevalue);
    }

    RPC_LOG("set_calc_aircondition");
    m_repeatComm.SetCalcAircondition(calc);
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(set_calc_power_gas)
{
    std::vector<xmlrpc_c::value> value = paramList.getArray(0, 12, 12);
    paramList.verifyEnd(1);
    std::vector<double> calc;
    for (std::vector<xmlrpc_c::value>::iterator it=value.begin();
                                                it!=value.end();
                                                it++)
    {
        double const doublevalue(static_cast<double>(xmlrpc_c::value_double(*it)));
        calc.push_back(doublevalue);
    }

    RPC_LOG("set_calc_power_gas");
    m_repeatComm.SetCalcPowerGas(calc);
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(set_calc_water)
{
    std::vector<xmlrpc_c::value> value = paramList.getArray(0, 12, 12);
    paramList.verifyEnd(1);
    std::vector<double> calc;
    for (std::vector<xmlrpc_c::value>::iterator it=value.begin();
                                                it!=value.end();
                                                it++)
    {
        double const doublevalue(static_cast<double>(xmlrpc_c::value_double(*it)));
        calc.push_back(doublevalue);
    }

    RPC_LOG("set_calc_water");
    m_repeatComm.SetCalcWater(calc);
    *retvalP = xmlrpc_c::value_int(1);

}
METHOD_END

METHOD_DEF(set_price)
{
    RPC_LOG("set_price");
    std::vector<xmlrpc_c::value> value = paramList.getArray(0, 6, 6);
    paramList.verifyEnd(1);
    std::vector<double> calc;
    for (std::vector<xmlrpc_c::value>::iterator it=value.begin();
                                                it!=value.end();
                                                it++)
    {
        double const doublevalue(static_cast<double>(xmlrpc_c::value_double(*it)));
        calc.push_back(doublevalue);
    }

    m_repeatComm.SetPrice(calc);
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(get_xinfeng_windspeed)
{
    RepeatComm::xinfeng_windspeed speed = m_repeatComm.GetXinfengWindSpeed(); 
    RPC_LOG("get_xinfeng_windspeed: %f %f", speed.windspeed, speed.area);
    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["windspeed"] =  xmlrpc_c::value_double(speed.windspeed);
    rpc_struct["area"] = xmlrpc_c::value_double(speed.area);
    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(set_xinfeng_windarea)
{
    double area = paramList.getDouble(0);
    paramList.verifyEnd(1);
    RPC_LOG("set_xinfeng_windarea: %f", area);
    m_repeatComm.SetXinfengWindarea(area);
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

// 设置室外环境参数
METHOD_DEF(set_outside_env)
{
    RPC_LOG("set_outside_env");
    std::vector<xmlrpc_c::value> value = paramList.getArray(0, 6, 6);
    paramList.verifyEnd(1);
    std::vector<double> data;
    for (std::vector<xmlrpc_c::value>::iterator it=value.begin();
                                                it!=value.end();
                                                it++)
    {
        double const doublevalue(static_cast<double>(xmlrpc_c::value_double(*it)));
        data.push_back(doublevalue);
    }

    m_repeatComm.SetOutsideEnv(data);
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

// 设置室外传感器参数
METHOD_DEF(set_outside_sen)
{
    RPC_LOG("set_outside_sen");
    std::vector<xmlrpc_c::value> value = paramList.getArray(0, 10, 10);
    paramList.verifyEnd(1);
    std::vector<int> data;
    for (std::vector<xmlrpc_c::value>::iterator it=value.begin();
                                                it!=value.end();
                                                it++)
    {
        int const intvalue(static_cast<int>(xmlrpc_c::value_int(*it)));
        data.push_back(intvalue);
    }

    m_repeatComm.SetOutsideSen(data);
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(set_inner_airdetection)
{
    RPC_LOG("set_inner_airdetection");
    m_repeatComm.SetInnerAirDetection();
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(set_timesyn_internet)
{
    RPC_LOG("set_timesyn_internet");
    int nTimeSynInternet = paramList.getInt(0);
    paramList.verifyEnd(1);
    m_repeatComm.SetTimeSynInternet(nTimeSynInternet);
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(get_timesyn_internet)
{
    RPC_LOG("get_timesyn_internet");
    int nTimeSynInternet = m_repeatComm.GetTimeSynInternet();
    *retvalP = xmlrpc_c::value_int(nTimeSynInternet);
}
METHOD_END


///////////////////////////////////////////////////////////////
// 节能模式RPC
//
METHOD_DEF(query_light_offtime)
{
    RPC_LOG("query_light_offtime");
    int nTime = m_repeatComm.QueryLightOffTime();
    *retvalP = xmlrpc_c::value_int(nTime);
}
METHOD_END

METHOD_DEF(set_light_offtime)
{
    RPC_LOG("set_lightoff_time");
    int index = paramList.getInt(0);
    paramList.verifyEnd(1);
    if (!m_repeatComm.SetLightOffTime(index))
    {
        throw(xmlrpc_c::fault("invalid index", (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));
    }
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(query_curtain_offtime)
{
    RPC_LOG("query_curtain_offtime");
    int nTime = m_repeatComm.QueryCurtainOffTime();
    *retvalP = xmlrpc_c::value_int(nTime);
}
METHOD_END

METHOD_DEF(set_curtain_offtime)
{
    RPC_LOG("set_curtain_offtime");
    int index = paramList.getInt(0);
    paramList.verifyEnd(1);
    if (!m_repeatComm.SetCurtainOffTime(index))
    {
        throw(xmlrpc_c::fault("invalid index", (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));
    }
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(query_dust_time)
{
    RPC_LOG("query_dust_time");
    int nTime = m_repeatComm.QueryDustTime();
    *retvalP = xmlrpc_c::value_int(nTime);
}
METHOD_END

METHOD_DEF(set_dust_time)
{
    RPC_LOG("set_dust_time");
    int index = paramList.getInt(0);
    paramList.verifyEnd(1);
    if (!m_repeatComm.SetDustTime(index))
    {
        throw(xmlrpc_c::fault("invalid index", (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));
    }
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

//////////////////////////////////////////////////////
// 报警

METHOD_DEF(query_inner_alarm)
{
    RPC_LOG("query_inner_alarm");
    std::vector<int> vec = m_repeatComm.QueryInnerAlarm();
    *retvalP = xmlrpc_c::arrayValueSlice(vec.begin(), vec.end());
}
METHOD_END

METHOD_DEF(query_inner_alarm_new)
{
    RPC_LOG("query_inner_alarm_new");
    std::vector<int> vec = m_repeatComm.QueryInnerAlarmNew();
    *retvalP = xmlrpc_c::arrayValueSlice(vec.begin(), vec.end());
}
METHOD_END

METHOD_DEF(query_outside_alarm)
{
    RPC_LOG("query_outside_alarm");
    std::vector<int> vec = m_repeatComm.QueryOutsideAlarm();
    *retvalP = xmlrpc_c::arrayValueSlice(vec.begin(), vec.end());
}
METHOD_END

/////////////////////////////////////////////////////////
// 新风时间控制
METHOD_DEF(query_xinfeng_timectrl)
{
    RPC_LOG("query_xinfeng_timectrl");
    RepeatComm::XinfengTimeCtrl time = m_repeatComm.QueryXinfengTimeCtrl();
    xmlrpc_c::cstruct rpc_struct;
    rpc_struct["nLeave"] = xmlrpc_c::value_int(time.nLeave);
    rpc_struct["nAutoRun"] =  xmlrpc_c::value_int(time.nAutoRun);
    *retvalP = xmlrpc_c::value_struct(rpc_struct);
}
METHOD_END

METHOD_DEF(set_xinfeng_timectrl)
{
    RepeatComm::XinfengTimeCtrl time;
    time.nLeave = paramList.getInt(0);
    time.nAutoRun = paramList.getInt(1);
    paramList.verifyEnd(2);

    RPC_LOG("set_xinfeng_timectrl nLeave:%d nAutoRun:%d", time.nLeave, time.nAutoRun);
    if (!m_repeatComm.SetXinfengTimeCtrl(time))
    {
        throw(xmlrpc_c::fault("invalid time", (xmlrpc_c::fault::code_t)RPC_METHOD_CALL_RETURN_FAIL));
    }
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END


//////////////////////////////////////////////////////
//
METHOD_DEF(set_time)
{
    //SetTime(int year, int month, int day, int hour, int minute, int second); 
    int year = paramList.getInt(0);
    int month = paramList.getInt(1);
    int day = paramList.getInt(2);
    int hour = paramList.getInt(3);
    int minute = paramList.getInt(4);
    int second = paramList.getInt(5);
    paramList.verifyEnd(6);

    RPC_LOG("set_time %d %d %d %d %d %d", year, month, day, hour, minute, second);

    if (m_repeatComm.SetTime(year, month, day, hour, minute, second))
    {
        *retvalP = xmlrpc_c::value_int(1);
    }
    else
    {
        *retvalP = xmlrpc_c::value_int(0);
    }
}
METHOD_END

METHOD_DEF(set_fire_alarm)
{
    int nAlarm = paramList.getInt(0);
    paramList.verifyEnd(1);
    RPC_LOG("set_fire_alarm %d", nAlarm);
    m_repeatComm.SetFireAlarm(nAlarm);
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(set_device_count)
{
    RPC_LOG("set_device_count");
    int nLight = paramList.getInt(0);
    int nCurtain = paramList.getInt(1);
    int nMeter = paramList.getInt(2);
    int nXinfeng = paramList.getInt(3);
    paramList.verifyEnd(4);
    m_repeatComm.SetDeviceCount(nLight, nCurtain, nMeter, nXinfeng);
    *retvalP = xmlrpc_c::value_int(1);
}
METHOD_END

METHOD_DEF(protocol_version_is_ok)
{
    int ret = 1;
    int major = paramList.getInt(0);
    int minor = paramList.getInt(1);
    //int release = paramList.getInt(2);
    paramList.verifyEnd(3);

    if (major != MAJOR)
    {
        ret = 0;
    }
    else if (minor > MINOR)
    {
        ret = 0;
    }

    *retvalP = xmlrpc_c::value_int(ret);
}
METHOD_END

METHOD_DEF(get_version)
{
    *retvalP = xmlrpc_c::value_string(VERSION);
}
METHOD_END

////////////////////////////////////////////////////////////////////////////////
#define ADD_METHOD(x) xmlrpc_c::methodPtr const p##x(new C##x(repeatComm)); \
    myRegistry.addMethod(#x, p##x)

int rpc_method_init(xmlrpc_c::registry &myRegistry, RepeatComm &repeatComm)
{
    ADD_METHOD(protocol_version_is_ok);

    ADD_METHOD(light_switch);
    ADD_METHOD(query_light_switch);
    ADD_METHOD(inner_curtain);
    ADD_METHOD(outside_curtain);
    ADD_METHOD(query_curtain);
    ADD_METHOD(query_curtain_verbose);
    ADD_METHOD(query_meter);
    ADD_METHOD(aircondition);
    ADD_METHOD(aircondition_svm);
    ADD_METHOD(query_aircondition);
    ADD_METHOD(aircondition_set_mode);

    // 带警报的查询
    ADD_METHOD(query_aircondition_verbose);

    ADD_METHOD(humansen_to_dev);
    ADD_METHOD(is_bind_change);
    ADD_METHOD(get_sen_status);
    ADD_METHOD(get_light_off_time);
    ADD_METHOD(get_device_count);
    ADD_METHOD(get_floor_room);
    ADD_METHOD(get_air_quality);
    ADD_METHOD(get_xinfeng_time);
    ADD_METHOD(get_xinfeng_switch_status);
    ADD_METHOD(set_device_count);
    ADD_METHOD(set_calc_aircondition);
    ADD_METHOD(set_calc_power_gas);
    ADD_METHOD(set_calc_water);
    ADD_METHOD(set_price);

    ADD_METHOD(get_xinfeng_windspeed);
    ADD_METHOD(set_xinfeng_windarea);

    ADD_METHOD(set_outside_env);
    ADD_METHOD(set_outside_sen);
    ADD_METHOD(set_inner_airdetection);

    ADD_METHOD(set_timesyn_internet);
    ADD_METHOD(get_timesyn_internet);

    ADD_METHOD(query_light_offtime);
    ADD_METHOD(set_light_offtime);
    ADD_METHOD(query_curtain_offtime);
    ADD_METHOD(set_curtain_offtime);
    ADD_METHOD(query_dust_time);
    ADD_METHOD(set_dust_time);

    ADD_METHOD(query_inner_alarm);
    ADD_METHOD(query_inner_alarm_new);
    ADD_METHOD(query_outside_alarm);

    ADD_METHOD(query_xinfeng_timectrl);
    ADD_METHOD(set_xinfeng_timectrl);

    ADD_METHOD(set_time);

    ADD_METHOD(set_fire_alarm);

    ADD_METHOD(get_version);
    return 0;
}
