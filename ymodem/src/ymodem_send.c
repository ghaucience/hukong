#include <sys/types.h>
#include <unistd.h>
#include <errno.h>  
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h> 
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <libgen.h>
#include "config.h"
#include "ymodem.h"

static int g_fd = -1;
static int open_serialport(const char *, int, const char *);
static int parse_setting(struct termios *, const char *);

void usage(const char *pstrProgram)
{
    printf("Ymodem upgrade tool for hukongqi\n\n");
    printf("Usage:\n"
           "  %s [opt] upgrade_filename\n"
           "    -d /dev/ttyXXX serial port dev file\n"
           "    -t             set /dev/ttyxxx baudrate etc, like stty\n"
           "    -h             show this help\n"
           "    -v             show version\n"
           "\n"
           "Example:\n"
           "    %s -d /dev/ttyUSB0  STM_upgrade_file\n",
           pstrProgram, pstrProgram);
}

int main(int argc, char *argv[])
{
    int opt = -1;
    const char *pstrSerial = NULL;
    char *pstrFilename = NULL;
    int nBaudrate = 9600;
    int bSetTTY = 0;
    while ((opt = getopt(argc, argv, "d:thv")) != -1)
    {
        switch (opt)
        {
            case 'd':
                pstrSerial = optarg;
                break;
            case 't':
                bSetTTY = 1;
                break;
            case 'h':
                usage(argv[0]);
                return 0;
            case 'v':
                printf("version: %s\n", VERSION);
                return 0;
            default: /* '?' */
                usage(argv[0]);
                return -1;
        }
    } 
    if (optind < argc)
    {
        pstrFilename = argv[optind];
    }
    else
    {
        if (!bSetTTY)
        {
            usage(argv[0]);
            return -1;
        }
    }

    if (pstrSerial == NULL)
    {
        usage(argv[0]);
        return -1;
    }

    g_fd = open_serialport(pstrSerial, nBaudrate, "8E1");
    if (g_fd == -1)
    {
        fprintf(stderr, "open serialport fail\n");
        return -1;
    }

    if (bSetTTY)
    {
        // set tty param and exit
        close(g_fd);
        g_fd = -1;
        return 0;
    }

    FILE *pFile = fopen(pstrFilename, "rb");
    if (pFile == NULL)
    {
        fprintf(stderr, "open file fail\n");
        return -1;
    }

    int fd = fileno(pFile);
    if (fd == -1)
    {
        fprintf(stderr, "open file fail\n");
        return -1;
    }

    struct stat stat_buf;
    if (fstat(fd, &stat_buf) == -1 || stat_buf.st_size == 0)
    {
        fprintf(stderr, "get file size fail\n");
        return -1;
    }

    unsigned char *pBuffer = malloc(stat_buf.st_size);
    memset(pBuffer, 0, stat_buf.st_size);
    printf("read %ld byte from %s\n", stat_buf.st_size, pstrFilename);
    long lLen = fread(pBuffer, 1, stat_buf.st_size, pFile);
    if (lLen == 0)
    {
        if (ferror(pFile))
        {
            fprintf(stderr, "fread file fail\n");
            free(pBuffer);
            pBuffer = NULL;
            return -1;
        }
    }
    fclose(pFile);
    pFile = NULL;

    printf("ready to send %ld byte\n", lLen);
    char *pstrBasename =  basename(pstrFilename);
    int nRet = 0;
    if (strlen(pstrBasename) > 0)
    {
        nRet = ymodem_send(pBuffer, lLen, pstrBasename);
    }
    else
    {
        fprintf(stderr, "file invalid, basename is empty\n");
    }

    free(pBuffer);
    pBuffer = NULL;

    close(g_fd);
    g_fd = -1;

    return (nRet > 0) ? 0:1;
}

void _flush_read()
{
    if (g_fd >= 0)
    {
        tcflush(g_fd, TCIOFLUSH);
    }
}

void _putchar(int c)
{
    if (g_fd >= 0)
    {
        unsigned char btData = (unsigned int)c;
        int nRet = write(g_fd, &btData, 1);
        if (nRet == -1)
        {
            fprintf(stderr, "write fail. strerr:%s\n", strerror(errno));
        }
    }
}

void _putchars(unsigned char *pchs, int nLen)
{
    if (g_fd >= 0)
    {
        int nRet = write(g_fd, pchs, nLen);
        if (nRet == -1)
        {
            fprintf(stderr, "write fail. strerr:%s\n", strerror(errno));
        }
    }
}

int _getchar(int timeout)
{
    fd_set fds;
    // timeout
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(g_fd, &fds);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    int nRet = select(g_fd+1, &fds, NULL, NULL, (timeout>=0)?&tv:NULL);
    if (nRet < 0)
    {
        fprintf(stderr, "select fail.\n");
        exit(-1);
    }
    else if (nRet == 0)
    {
        //timeout
        return -1;
    }
    else
    {
        int nLen;
        if (ioctl(g_fd, FIONREAD, &nLen) >= 0)
        {
            if (nLen > 0)
            {
                unsigned char btData = 0;
                int nRet = read(g_fd, &btData, 1);
                if (nRet == -1 )
                {
                    fprintf(stderr, "read fail\n");
                    exit(-1);
                }
                else if (nRet == 0)
                {
                    // end file
                    return -1;
                }
                else
                {
                    return btData;
                }
            }
        }
        else
        {
            fprintf(stderr, "serialport ioctl get byte available err. strerr:%s\n", strerror(errno));
            exit(-1);
            return -1;
        }
    }
    return -1;
}

// open serialport
static int open_serialport(const char *pstrPort, int nBaudRate, const char *pstrSetting)
{
    // O_NONBLOCK: read will return immediatly
    int fd = open(pstrPort, O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        fprintf(stderr, "serialport open fail!\n");
        return -1;
    }

    // serial port setting
    struct termios options;
    bzero(&options, sizeof(options));

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
            fprintf(stderr, "serialport wrong baudrate %d\n", nBaudRate);
            return -1;
    }

    options.c_cflag |= (CLOCAL | CREAD);
    // raw input
    options.c_lflag = 0;
    // raw input
    options.c_iflag = 0;
    // raw output
    options.c_oflag = 0;

    if (parse_setting(&options, pstrSetting) == -1)
    {
        fprintf(stderr, "serialport pstrSetting invalid:%s", pstrSetting);
        close(fd);
        return -1;
    }

    if (tcsetattr(fd, TCSANOW, &options) < 0)
    {
        fprintf(stderr, "serialport tcsetattr fail.\n");
        close(fd);
        return -1;
    }

    tcflush(fd, TCIOFLUSH);
    return fd;
}

// parse serial setting, 8N1...
static int parse_setting(struct termios *pOptions, const char *pstrSetting)
{
    if (strlen(pstrSetting) == 3)
    {
        // Mask the character size bits
        pOptions->c_cflag &= ~CSIZE; 
        switch (pstrSetting[0])
        {
            case '5':
                pOptions->c_cflag |= CS5;
                break;
            case '6':
                pOptions->c_cflag |= CS6;
                break;
            case '7':
                pOptions->c_cflag |= CS7;
                break;
            case '8':
                pOptions->c_cflag |= CS8;
                break;
            default:
                return -1;
        }

        // parity setting
        switch (pstrSetting[1])
        {
            case 'N':
                pOptions->c_cflag &= ~PARENB;
                break;
            case 'E':
                pOptions->c_cflag |= PARENB;
                pOptions->c_cflag &= ~PARODD;
                // enable parity input check
                pOptions->c_iflag |= (INPCK | ISTRIP);
                break;
            case 'O':
                pOptions->c_cflag |= PARENB;
                pOptions->c_cflag |= PARODD;
                // enable parity input check
                pOptions->c_iflag |= (INPCK | ISTRIP);
                break;
            default:
                return -1;
        }

        // stopbit
        switch (pstrSetting[2])
        {
            case '1':
                pOptions->c_cflag &= ~CSTOPB;
                break;
            case '2':
                pOptions->c_cflag |= CSTOPB;
                break;
            default:
                return -1;
        }

        return 0;
    }

    return -1;
}
