#ifndef _SNET_SYSTEM_H_
#define _SNET_SYSTEM_H_

namespace lib_linux
{
	namespace system
	{
		// 空操作，保证调用后连接器连接这个Obj，从而初始化NetworkInitialize
		void Initial(void);
	}

}

#endif
