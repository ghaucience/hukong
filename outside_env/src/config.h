#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MAJOR   0
#define MINOR   1
#define RELEASE 8
#define EXTRA   dev

#define _STR(x)  #x
#define STR(x)  _STR(x)
#define VERSION  STR(MAJOR)"."STR(MINOR)"."STR(RELEASE)" "STR(EXTRA)

#define RPC_METHOD_CALL_RETURN_FAIL  -32500

typedef  unsigned char   BYTE;
typedef  unsigned short  WORD;
typedef  unsigned int    DWORD;

#define GET_BIT(byte, bitseq)              (((byte)>>bitseq)&1)
#define SET_BIT(byte, bitseq, value)       (((byte)&~(1<<bitseq))|(((value)&1)<<bitseq))
#define GET_BITS(byte, begin, end)         (((byte)>>begin)&((1<<(end-begin+1))-1))
#define SET_BITS(byte, begin, end, value)  (((byte)&~(((1<<(end-begin+1))-1)<<begin))|((value)<<begin))

#endif
