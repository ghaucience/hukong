#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "rpc_client.h"
#include "rpc_config.h"

#define NAME "XMLRPC hukong Client"
#define VERSION "0"

extern struct xmlrpc_client_transport_ops xmlrpc_curl_transport_ops;

int rpc_client_init(rpc_client *pRPC)
{
    memset(pRPC, 0, sizeof(rpc_client));
    xmlrpc_env_init(&pRPC->m_env);

    xmlrpc_env envTemp;
    xmlrpc_env_init(&envTemp);
    xmlrpc_client_setup_global_const(&envTemp);
    if (envTemp.fault_occurred)
    {
        xmlrpc_env_clean(&envTemp);
        fprintf(stderr, "init curl fail\n");
        return -1;
    }

    // Start up our XML-RPC client library. Now use libcurl
    pRPC->m_pClient = NULL;
    xmlrpc_client_create(&envTemp, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0, &pRPC->m_pClient);
    if (envTemp.fault_occurred)
    {
        xmlrpc_env_clean(&envTemp);
        fprintf(stderr, "create xmlrpc client fail: %s (%d)\n", envTemp.fault_string, envTemp.fault_code);
        return -1;
    }

    return 0;
}

void rpc_client_destroy(rpc_client *pRPC)
{
    if (pRPC->m_pstrSave != NULL)
    {
        free((char *)(pRPC->m_pstrSave));
        pRPC->m_pstrSave = NULL;
    }

    if (pRPC->m_pstrURL)
    {
        free(pRPC->m_pstrURL);
        pRPC->m_pstrURL = NULL;
    }

    xmlrpc_env_clean(&pRPC->m_env);    

    // shutdown our XML-RPC client library.
    xmlrpc_client_destroy(pRPC->m_pClient);
    pRPC->m_pClient = NULL;
}

void rpc_client_set_url(rpc_client *pRPC, const char *pstrURL)
{
    assert(pstrURL);
    if (pRPC->m_pstrURL)
    {
        free(pRPC->m_pstrURL);
        pRPC->m_pstrURL = NULL;
    }

    pRPC->m_pstrURL = (char *)malloc(strlen(pstrURL) + 1);
    strcpy(pRPC->m_pstrURL, pstrURL);
}

int rpc_is_call_ok(rpc_client *pRPC)
{
    return pRPC->m_env.fault_occurred?0:1;
}

void rpc_print_error(rpc_client *pRPC)
{
    if (!rpc_is_call_ok(pRPC))
    {
        printf("call fault: %d %s\n", pRPC->m_env.fault_code, pRPC->m_env.fault_string);
    }
}

static const char *rpc_client_getstring(rpc_client *pRPC, xmlrpc_value *result)
{
    if (pRPC->m_pstrSave != NULL)
    {
        free((char *)(pRPC->m_pstrSave));
        pRPC->m_pstrSave = NULL;
    }

    xmlrpc_read_string(&pRPC->m_env, result, &pRPC->m_pstrSave);
    return pRPC->m_pstrSave;
}

static xmlrpc_value *rpc_client_call(rpc_client *pRPC, const char *pstrMethod, const char *format, ...)
{
    assert(pRPC->m_pstrURL);

    // init pRPC's env, because every call must has a clean env.
    xmlrpc_env_clean(&pRPC->m_env);    
    xmlrpc_env_init(&pRPC->m_env);

    xmlrpc_value * resultP = NULL;
    va_list args;
    va_start(args, format);
    xmlrpc_client_call2f_va(&pRPC->m_env, pRPC->m_pClient, pRPC->m_pstrURL,
            pstrMethod, format, &resultP, args);
    va_end(args);
    return resultP;
}

//////////////////////////////////////////////////////////
// RPC call implement

const char *protocol_version(rpc_client *pRPC)
{
    return RPC_VERSION;
}

int protocol_version_is_ok(rpc_client *pRPC)
{
    int ret = 0;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "(iii)", RPC_MAJOR, RPC_MINOR, RPC_RELEASE);
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_read_int(&pRPC->m_env, result, &ret);
        xmlrpc_DECREF(result);
    }
    return ret;
}


void light_switch(rpc_client *pRPC, int nDestAddr, int nIndex, int mode)
{
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "(iii)", nDestAddr, nIndex, mode);
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_DECREF(result);
    }
}

light_switch_status query_light_switch(rpc_client *pRPC, int nDestAddr)
{
    light_switch_status status;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "(i)", nDestAddr);
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_decompose_value(&pRPC->m_env, result, 
                               "{s:i,s:i,s:i,s:i,s:i,s:i,*}",
                               "light1_status", &status.light1_status,
                               "light2_status", &status.light2_status,
                               "light3_status", &status.light3_status,
                               "light4_status", &status.light4_status,
                               "light5_status", &status.light5_status,
                               "light6_status", &status.light6_status
                               );
        xmlrpc_DECREF(result);
    }
    return status;
}

void curtain_switch(rpc_client *pRPC, int nDestAddr, int index, int mode)
{
    xmlrpc_value *result = NULL;
    if (index == 0)
    {
        result = rpc_client_call(pRPC, "inner_curtain", "(ii)", nDestAddr, mode);
    }
    else
    {
        result = rpc_client_call(pRPC, "outside_curtain", "(ii)", nDestAddr, mode);
    }

    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_DECREF(result);
    }
}

curtain_status query_curtain(rpc_client *pRPC, int nDestAddr)
{
    curtain_status status;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "(i)", nDestAddr);
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_decompose_value(&pRPC->m_env, result, 
                               "{s:i,s:i,s:i,s:i,*}",
                               "inner", &status.inner,
                               "outside", &status.outside,
                               "inner_fault", &status.inner_fault,
                               "outside_fault", &status.outside_fault
                               );
        xmlrpc_DECREF(result);
    }
    return status;
}

meter_status query_meter(rpc_client *pRPC, int nDestAddr)
{
    meter_status status;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "(i)", nDestAddr);
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_decompose_value(&pRPC->m_env, result, 
                               "{s:d,s:d,s:d,s:d,s:d,s:d,s:d,s:d,s:d,s:d,s:d,s:d,*}",
                               "cold_cur_rate", &status.cold_cur_rate,
                               "hot_cur_rate", &status.hot_cur_rate,
                               "cold_totoal", &status.cold_totoal,
                               "hot_totoal", &status.hot_totoal,
                               "hot_input_temp", &status.hot_input_temp,
                               "hot_output_temp", &status.hot_output_temp,
                               "hot_totoal_degree", &status.hot_totoal_degree,
                               "hot_cur_power", &status.hot_cur_power,
                               "cur_totoal_power", &status.cur_totoal_power,
                               "forward_totoal_degree", &status.forward_totoal_degree,
                               "gas_cur_rate", &status.gas_cur_rate,
                               "gas_totoal_rate", &status.gas_totoal_rate
                               );
        xmlrpc_DECREF(result);
    }
    return status;
}

void aircondition(rpc_client *pRPC, int nDestAddr, int switch_mode, int aircondition_mode, int xinfeng_mode, int temp, int energy_save_mode)
{
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "(iiiiii)", nDestAddr, switch_mode, aircondition_mode, xinfeng_mode, temp, energy_save_mode);
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_DECREF(result);
    }
}

aircondition_status query_aircondition(rpc_client *pRPC, int nDestAddr)
{
    aircondition_status status;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "(i)", nDestAddr);
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_decompose_value(&pRPC->m_env, result, 
                               "{s:i,s:i,s:i,s:i,s:i,*}",
                               "switch_mode", &status.switch_mode,
                               "aircondition_mode", &status.aircondition_mode,
                               "xinfeng_mode", &status.xinfeng_mode,
                               "temp_setting", &status.temp_setting,
                               "energy_save_mode", &status.energy_save_mode
                               );
        xmlrpc_DECREF(result);
    }
    return status;
}

/////////////////////////////////////////////////////////////////////////////////
int humansen_to_dev(rpc_client *pRPC, int nSenID, DevInfo *pInfo, int nLen)
{
    int size = 0;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "(i)", nSenID);
    if (rpc_is_call_ok(pRPC))
    {
        // array size
        size = xmlrpc_array_size(&pRPC->m_env, result);
        if (size > nLen)
        {
            xmlrpc_DECREF(result);
            return -1;
        }
        else
        {
            int i;
            for (i=0; i<size; i++)
            {
                int value;
                xmlrpc_value *pIntValue = NULL;
                xmlrpc_array_read_item(&pRPC->m_env, result, i, &pIntValue);
                xmlrpc_read_int(&pRPC->m_env, pIntValue, &value);
                xmlrpc_DECREF(pIntValue);

                pInfo[i].type = value>>16;
                pInfo[i].addr = (value&0xFFFF)>>8;
                pInfo[i].opt = value&0xFF;
            }
        }
        xmlrpc_DECREF(result);
    }
    return size;
}

int is_bind_change(rpc_client *pRPC)
{
    xmlrpc_bool ret = 0;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "()");
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_read_bool(&pRPC->m_env, result, &ret);
        xmlrpc_DECREF(result);
    }
    return ret;
}

int get_sen_status(rpc_client *pRPC, int *pData, int nLen)
{
    int size = 0;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "()");
    if (rpc_is_call_ok(pRPC))
    {
        // array size
        size = xmlrpc_array_size(&pRPC->m_env, result);
        if (size > nLen)
        {
            xmlrpc_DECREF(result);
            return -1;
        }
        else
        {
            int i;
            for (i=0; i<size; i++)
            {
                int value;
                xmlrpc_value *pIntValue = NULL;
                xmlrpc_array_read_item(&pRPC->m_env, result, i, &pIntValue);
                xmlrpc_read_int(&pRPC->m_env, pIntValue, &value);
                xmlrpc_DECREF(pIntValue);
                pData[i] = value;
            }
        }
        xmlrpc_DECREF(result);
    }
    return size;
}

int get_light_off_time(rpc_client *pRPC)
{
    int ret = 0;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "()");
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_read_int(&pRPC->m_env, result, &ret);
        xmlrpc_DECREF(result);
    }
    return ret;
}

int get_device_count(rpc_client *pRPC, DevCount *pCount, int nLen)
{
    int size = 0;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "()");
    if (rpc_is_call_ok(pRPC))
    {
        // array size
        size = xmlrpc_array_size(&pRPC->m_env, result);
        if (size > nLen)
        {
            xmlrpc_DECREF(result);
            return -1;
        }
        else
        {
            int i;
            for (i=0; i<size; i++)
            {
                xmlrpc_value *pStruct = NULL;
                xmlrpc_array_read_item(&pRPC->m_env, result, i, &pStruct);
                xmlrpc_decompose_value(&pRPC->m_env, pStruct, 
                                       "{s:i,s:i,*}",
                                       "type", &(pCount[i].type),
                                       "count", &(pCount[i].count)
                                       );
                xmlrpc_DECREF(pStruct);
            }
        }
        xmlrpc_DECREF(result);
    }
    return size;
}

air_quality get_air_quality(rpc_client *pRPC)
{
    air_quality air;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "()");
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_decompose_value(&pRPC->m_env, result, 
                               "{s:i,s:i,s:i,s:d,s:i,s:i,s:i,*}",
                               "inner_PM03", &air.inner_PM03,
                               "inner_PM25", &air.inner_PM25,
                               "inner_PM10", &air.inner_PM10,
                               "inner_VOC", &air.inner_VOC,
                               "inner_CO2", &air.inner_CO2,
                               "inner_temp", &air.inner_temp,
                               "inner_humidity", &air.inner_humidity
                               );
        xmlrpc_DECREF(result);
    }
    return air;
}

xinfeng_time get_xinfeng_time(rpc_client *pRPC)
{
    xinfeng_time times;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "()");
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_decompose_value(&pRPC->m_env, result, 
                               "{s:i,s:i,*}",
                               "nOFF", &times.nOFF,
                               "nON", &times.nON
                               );
        xmlrpc_DECREF(result);
    }
    return times;
}

xinfeng_switch_status get_xinfeng_switch_status(rpc_client *pRPC, int nDestAddr)
{
    xinfeng_switch_status status;
    xmlrpc_value *result = rpc_client_call(pRPC, __FUNCTION__, "(i)", nDestAddr);
    if (rpc_is_call_ok(pRPC))
    {
        xmlrpc_decompose_value(&pRPC->m_env, result, 
                               "{s:i,s:i,*}",
                               "nXinfengSwitch", &status.nXinfengSwitch,
                               "nTiaofengSwitch", &status.nTiaofengSwitch
                               );
        xmlrpc_DECREF(result);
    }
    return status;
}
