#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "rpc_client.h"
#include "rpc_config.h"
#include <time.h>

int main(int argc, char *argv[])
{
    int i;
    int nTemp;
    rpc_client client;

    int nRet = rpc_client_init(&client);
    if (nRet !=  0)
    {
        fprintf(stderr, "rpc init fail\n");
        return -1;
    }
    
    rpc_client_set_url(&client, "http://192.168.2.100:8000/RPC2");

    printf("client version:%s\n", protocol_version(&client));

    int ret = protocol_version_is_ok(&client);
    //assert(rpc_is_call_ok(&client));
    if (ret)
    {
        printf("version is ok\n");
    }
    else
    {
        printf("version is not ok!\n");
    }

    while (true)
    {
        light_switch(&client, 1, 0, ON);
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        ////assert(rpc_is_call_ok(&client));
        printf("light_switch ON\n\n");

        light_switch_status _status_light = query_light_switch(&client, 1);
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        //assert(rpc_is_call_ok(&client));
        printf("query_light_switch\n\n");

        curtain_switch(&client, 1, 1, ON);
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        ////assert(rpc_is_call_ok(&client));
        printf("curtain_switch outside Open\n\n");


        curtain_status _status_curtain;
        query_curtain(&client, 1);
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        //assert(rpc_is_call_ok(&client));
        printf("query_curtain \n\n");


        meter_status _status_meter;
        query_meter(&client, 1);
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        //assert(rpc_is_call_ok(&client));
        printf("query_meter \n\n");

        //aircondition 纯新风
        aircondition(&client, 1, ON, MODE_COLD, 1, 20, 4); 
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        ////assert(rpc_is_call_ok(&client));
        printf("aircondition ON MODE_COLD 1 20\n\n");

        // 查询空调状态
        aircondition_status _aircondition_status = query_aircondition(&client, 1);
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        //assert(rpc_is_call_ok(&client));
        printf("query_aircondition \n\n");


        /////////////////////////////////////////////////////////////////////////////
        DevInfo info[256];
        int nCount = humansen_to_dev(&client, 1, info, sizeof(info)/sizeof(info[0]));
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        //assert(rpc_is_call_ok(&client));
        printf("humansen_to_dev humanID:%d count:%d\n\n", 1, nCount);
        // 现在只返回灯光和窗帘绑定，默认每个都绑定到人感1
        //assert(nCount == 1);
        //assert(info[0].type == TYPE_LIGHT);
        //assert(info[0].addr == 1);
        //assert(info[0].opt == 1);
        // 测试错误
        nCount = humansen_to_dev(&client, 1, info, 0);
        //assert(nCount == -1);

        nRet = is_bind_change(&client);
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        //assert(rpc_is_call_ok(&client));
        printf("is_bind_change: %d\n\n", nRet);
        //assert(nRet == false);

        int sen_status[256];
        nCount = get_sen_status(&client, sen_status, sizeof(sen_status)/sizeof(sen_status[0]));
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        //assert(rpc_is_call_ok(&client));
        printf("get_sen_status count:%d\n", nCount);
        for (i=0; i<nCount; i++)
        {
            printf("%d ", sen_status[i]);
        }
        printf("\n\n");
        // 测试错误
        nCount = get_sen_status(&client, sen_status, 0);
        //assert(nCount == -1);

        int nTime = get_light_off_time(&client);
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        //assert(rpc_is_call_ok(&client));
        printf("get_light_off_time: %d\n\n", nTime);
        //assert(nTime == 2*60 || nTime == 2*60*60);

        DevCount devCount[256];
        nCount = get_device_count(&client, devCount, sizeof(devCount)/sizeof(devCount[0]));
        if (!rpc_is_call_ok(&client))
        {
            rpc_print_error(&client);
        }
        //assert(rpc_is_call_ok(&client));
        printf("get_device_count count:%d\n\n", nCount);
        // 测试错误
        nCount = get_device_count(&client, devCount, 0);
        //assert(nCount == -1);

        //struct timespec tm = { 0, 50000000};
        //nanosleep(&tm, NULL);
    }

    rpc_client_destroy(&client);
    return 0;
}
