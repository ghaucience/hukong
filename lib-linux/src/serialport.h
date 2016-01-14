#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__
#include "thread_wrap.h"
#include <termios.h> 
#include <unistd.h> 
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>  
#include <string>

#include "lib_linux_config.h"

class Serialport:public lib_linux::Thread
{
    public:
        Serialport(bool bThreadRead=true)
            :m_fd(-1),
            m_bRunFlag(false),
            m_nBaudRate(0),
            m_bThreadRead(bThreadRead)
        {
        }

        ~Serialport()
        {
            if (IsOpen())
            {
                WARNING("must close serialport before exit");
                Close();
            }
        }

        void Close()
        {
            if (m_bThreadRead)
            {
                // waitting thread
                m_bRunFlag = false;
                Wait();
            }

            if (IsOpen())
            {
                ::close(m_fd);
                m_fd = -1;
            }
        }

        // open serialport
        bool Open(const char *pstrPort, int nBaudRate, const char *pstrSetting="8N1")
        {
            // O_NONBLOCK: read will return immediatly
            //m_fd = ::open(pstrPort, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
            m_fd = ::open(pstrPort, O_RDWR | O_NOCTTY);
            if (m_fd < 0)
            {
                ERROR("serialport open fail!");
                m_fd = -1;
                return false;
            }

            // serial port setting
            struct termios options;
            ::bzero(&options, sizeof(options));

            // set boudrate
            switch (nBaudRate)
            {
                case 2400:
                    options.c_cflag |= B2400;
                    break;
                case 4800:
                    options.c_cflag |= B4800;
                    break;
                case 9600:
                    options.c_cflag |= B9600;
                    break;
                case 19200:
                    options.c_cflag |= B19200;
                    break;
                case 38400:
                    options.c_cflag |= B38400;
                    break;
                case 115200:
                    options.c_cflag |= B115200;
                    break;
                default:
                    ERROR("serialport wrong baudrate %d\n", nBaudRate);
                    return false;
            }

            m_nBaudRate = nBaudRate;

            options.c_cflag |= (CLOCAL | CREAD);
            // raw input
            options.c_lflag = 0;
            // raw input
            options.c_iflag = 0;
            // raw output
            options.c_oflag = 0;

            if (!ParseSerialSetting(options, pstrSetting))
            {
                ERROR("serialport pstrSetting invalid:%s", pstrSetting);
                ::close(m_fd);
                m_fd = -1;
                return false;
            }

            if (::tcsetattr(m_fd, TCSANOW, &options) < 0)
            {
                ERROR("serialport tcsetattr fail");
                ::close(m_fd);
                m_fd = -1;
                return false;
            }

            // flush all read
            Flush();

            if (m_bThreadRead)
            {
                // open thread
                m_bRunFlag = true;
                Start();
            }
            return true;
        }

        int Read(void *Buffer, int nBufferLength)
        {
            assert(m_fd > 0 && nBufferLength > 0);
            int nRet = ::read(m_fd, Buffer, nBufferLength);
            if (nRet < 0)
            {
                ERROR("serialport read fail");
                return -1;
            }
            return nRet;
        }

        int Write(void *Buffer, int nBufferLength)
        {
            assert(m_fd > 0 && nBufferLength >= 0);
            if (nBufferLength > 0)
            {
                int nRet = ::write(m_fd, Buffer, nBufferLength);
                if (nRet < 0)
                {
                    ERROR("serialport write fail");
                    return -1;
                }
                return nRet;
            }
            else
            {
                return 0;
            }
        }

        void Flush()
        {
            if (IsOpen())
            {
                if (::tcflush(m_fd, TCIOFLUSH) < 0)
                {
                    ERROR("serialport tcflush fail");
                }
            }
        }

        inline bool IsOpen()
        {
            return m_fd > 0;
        }

        inline int GetBaudrate()
        {
            return m_nBaudRate;
        }

        inline int GetFD()
        {
            return m_fd;
        }

    protected:
        // thread run
        void Run()
        {
            int max_fd;
            fd_set fds;
            struct timeval timeout;
            assert(m_fd > 0);
            while (IsOpen())
            {
                FD_ZERO(&fds);
                FD_SET(m_fd, &fds);
                max_fd = m_fd + 1;

                // timeout 10ms
                timeout.tv_sec = 0;
                timeout.tv_usec = 10*1000;

                int nRet = ::select(max_fd, &fds, NULL, NULL, &timeout);

                // check running
                if (!m_bRunFlag)
                {
                    break;
                }

                if (nRet < 0)
                {
                    ERROR("serial port select fail");
                    exit(-1);
                }
                else if (nRet == 0)
                {
                    //timeout
                }
                else
                {
                    if (FD_ISSET(m_fd, &fds))
                    {
                        int nLen;
                        if (::ioctl(m_fd, FIONREAD, &nLen) >= 0)
                        {
                            if (nLen > 0)
                            {
                                OnReceive(nLen);
                            }
                        }
                        else
                        {
                            ERROR("serialport ioctl get byte available err");
                            exit(-1);
                        }
                    }
                    else
                    {
                        ERROR("serial port unexpect condition in select!");
                        exit(-1);
                    }
                }
            }
        }

        // parse serial setting, 8N1...
        bool ParseSerialSetting(struct termios &options, const char *pstrSetting)
        {
            if (strlen(pstrSetting) == 3)
            {
                // Mask the character size bits
                options.c_cflag &= ~CSIZE; 
                switch (pstrSetting[0])
                {
                    case '5':
                        options.c_cflag |= CS5;
                        break;
                    case '6':
                        options.c_cflag |= CS6;
                        break;
                    case '7':
                        options.c_cflag |= CS7;
                        break;
                    case '8':
                        options.c_cflag |= CS8;
                        break;
                    default:
                        return false;
                }

                // parity setting
                switch (pstrSetting[1])
                {
                    case 'N':
                        options.c_cflag &= ~PARENB;
                        break;
                    case 'E':
                        options.c_cflag |= PARENB;
                        options.c_cflag &= ~PARODD;
                        // enable parity input check
                        options.c_iflag |= (INPCK | ISTRIP);
                        break;
                    case 'O':
                        options.c_cflag |= PARENB;
                        options.c_cflag |= PARODD;
                        // enable parity input check
                        options.c_iflag |= (INPCK | ISTRIP);
                        break;
                    default:
                        return false;
                }

                // stopbit
                switch (pstrSetting[2])
                {
                    case '1':
                        options.c_cflag &= ~CSTOPB;
                        break;
                    case '2':
                        options.c_cflag |= CSTOPB;
                        break;
                    default:
                        return false;
                }

                return true;
            }

            return false;
        }

        // receive event
        virtual void OnReceive(int nLen)
        {
        }

    private:
        int m_fd;
        // control thread exit
        volatile bool m_bRunFlag;
        // baudrate
        int m_nBaudRate;
        // thread read
        bool m_bThreadRead;
};

#endif
