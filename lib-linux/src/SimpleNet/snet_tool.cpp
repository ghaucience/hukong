#if defined(_WIN32) && defined(_MSC_VER)
    #pragma comment(lib, "ws2_32.lib")
#endif

#include "snet_config.h"
#include "snet_tool.h"

namespace lib_linux
{
	namespace system
	{
		void Initial(void)
		{
		}
	}

	// 自动初始化winsock
    namespace
    {
        class NetworkInitializer
        {
            public:
                NetworkInitializer()
                {
#if defined(_WIN32)
                    WSADATA wd;
                    WSAStartup(MAKEWORD( 2, 2 ), &wd);
#endif
                }

                ~NetworkInitializer()
                {
#if defined(_WIN32)
                    WSACleanup();
#endif
                }
        };

        static NetworkInitializer networkInitializer;
    }

}
