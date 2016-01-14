//
//  @author   lbzhung
//  @brief    packet define
//  @date     2013.4.26
//
#ifndef __PROTOCOL_PACKET_DEF_H_
#define __PROTOCOL_PACKET_DEF_H_

// address
#define ADDR_MIN       0x00
#define ADDR_MAX       0x50
#define ADDR_BORADCAST 0x00

// type code
#define TYPE_KONGGUANG 0x14
#define TYPE_LIANGKONG 0x19
#define TYPE_XINFENG  0x1C
#define TYPE_HUKONG    0x1D
#define TYPE_DENGKONG  0x1E
#define TYPE_CHAOBIAO  0x1F

// function code
#define FUN_PARAM      0x01
#define FUN_SETTING    0x81
#define FUN_XINFENG_KONGTIAO 0x02
#define FUN_DIANLI     0x03
#define FUN_ZHILAISUI  0x04
#define FUN_REALTIME_DATA 0x05
#define FUN_ADDR_MODIFLY  0x88
#define FUN_ZHANGMING_DEBUG 0x31
#define FUN_CHANGLIANG_DEBUG 0x32
#define FUN_SHUANGFENG_DEBUG 0x33
#define FUN_HUKONG_SET  0x41
#define FUN_HUKONG_QUERY 0x42
#define FUN_LIGHT_SET   0x43
#define FUN_LIGHT_QUERY 0x44
#define FUN_CHUANGLIANG_SET 0x45
#define FUN_CHUANGLIANG_QUERY 0x46
#define FUN_SHUANGFENG_SET 0x47
#define FUN_SHUANGFENG_QUERY 0x48
#define FUN_CHAOBIAO_QUERY 0x49

#endif
