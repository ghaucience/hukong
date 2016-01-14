//
//  @author zlb
//  @date   2013.4.22
//  @brief  config file
//
#ifndef _PROTOCOL_CONFIG_H_
#define _PROTOCOL_CONFIG_H_
#include <stdio.h>

typedef  unsigned char   BYTE;
typedef  unsigned short  WORD;
typedef  unsigned int    DWORD;

// not portable, use __cpu_to_le16/__cpu_to_le32 
/*
#define SWAP16(A)             ((((WORD)(A) & 0xFF00) >> 8) | \
                              (((WORD)(A) & 0x00FF) << 8) )
#define SWAP32(A)             ((((DWORD)(A) & 0xff000000) >> 24) | \
                              (((DWORD)(A) & 0x00ff0000) >> 8) | \
                              (((DWORD)(A) & 0x0000ff00) << 8) | \
                              (((DWORD)(A) & 0x000000ff) << 24))
*/

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define GET_BIT(byte, bitseq)              (((byte)>>bitseq)&1)
#define SET_BIT(byte, bitseq, value)       (((byte)&~(1<<bitseq))|(((value)&1)<<bitseq))
#define GET_BITS(byte, begin, end)         (((byte)>>begin)&((1<<(end-begin+1))-1))
#define SET_BITS(byte, begin, end, value)  (((byte)&~(((1<<(end-begin+1))-1)<<begin))|((value)<<begin))

// TODO 调小了超时时间，抵消空管器收发数据产生的损耗
#define TIMEOUT 180

// hukong front panel led
#define LED1_GPIO 14
#define LED2_GPIO 15
#endif
