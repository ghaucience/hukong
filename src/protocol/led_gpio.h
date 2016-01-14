#ifndef __LED_GPIO_H__
#define __LED_GPIO_H__
#include <vector>
#include "utility.h"

class LedGPIO
{
    public:
        LedGPIO()
        :m_fd(-1)
        {
        }

        ~LedGPIO()
        {
            if (m_fd != -1)
            {
                close(m_fd);
            }
        }

        bool Open(int nGPIO)
        {
            assert(m_fd == -1);
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "/sys/class/gpio/gpio%d/value", nGPIO);
            int fd = open(buffer, O_RDWR);
            if (fd == -1)
            {
                ERROR("open led gpio %s fail", buffer);
                return false;
            }
            m_fd = fd;
            return true;
        }

        void Flash()
        {
            if (m_fd != -1)
            {
                write(m_fd, "1", 1);
                lib_linux::Utility::Sleep(5);
                write(m_fd, "0", 1);
            }
        }

        void Turn(bool bOn)
        {
            if (m_fd != -1)
            {
                write(m_fd, bOn?"1":"0", 1);
            }
        }

        bool Get()
        {
            if (m_fd != -1)
            {
                char data[1] = {0};
                lseek(m_fd, 0, SEEK_SET);
                if (read(m_fd, data, 1) == 1)
                {
                    return data[0] == '1';
                }
            }
            return false;
        }

    private:
        int m_fd;
};
#endif
