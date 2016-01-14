#include <iostream>
#include <string.h>
#include "lib_linux_config.h"
#include "serialport.h"
#include "utility.h"

class MySerialportTest: public Serialport
{
    protected:
        // receive event
        void OnReceive(int nLen)
        {
            char buffer[128];
            int nRead = Read(buffer, sizeof(buffer)-1);
            if (nRead > 0)
            {
                buffer[nRead] = '\0';
                std::cout<<buffer<<std::endl;
            }
        }
};

int main(const int argc, const char *argv[])
{
    SET_LOG(lib_linux::FLAG_CON|lib_linux::FLAG_COLOR, ERROR);
    if (argc != 2)
    {
        std::cout<<"usage:"<<std::endl;
        std::cout<<"    "<<argv[0]<<"  /dev/ttyXXX"<<std::endl;
        return 1;
    }

    MySerialportTest port;
    if (!port.Open(argv[1], 9600))
    {
        return -1;
    }

    while (true)
    {
        const char *pStr = "   Hello, World!   ";
        port.Write((void *)pStr, strlen(pStr));
        lib_linux::Utility::Sleep(100);
    }
    return 0;
}
