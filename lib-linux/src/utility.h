#ifndef __UTILITY_H__
#define __UTILITY_H__
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>
#include <string>
#include "lib_linux_config.h"

namespace lib_linux
{
    class Utility
    {
        public:
            // just copy from apue
            static bool Daemonize()
            {
                pid_t pid;
                //struct rlimit    rl;
                struct sigaction sa;

                // clear file creation mask
                umask(0);

                //if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
                //{
                //    return false;
                //}

                // become a session leader to lose controlling tty
                if ((pid = fork()) < 0)
                {
                    return false;
                }
                else if (pid != 0)
                {
                    // parent
                    exit(0);
                }

                // create session
                setsid();

                // ensure future opens won't allocate controlling ttys.
                sa.sa_handler = SIG_IGN;
                sigemptyset(&sa.sa_mask);
                sa.sa_flags = 0;
                if (sigaction(SIGHUP, &sa, NULL) < 0)
                {
                    return false;
                }
                if ((pid = fork()) < 0)
                {
                    return false;
                }
                else if (pid != 0)
                {
                    // parent
                    exit(0);
                }

                if (chdir("/") < 0)
                {
                    ERROR("chdir / fail\n");
                    return false;
                }

                // close all open file descriptors
                //if (rl.rlim_max == RLIM_INFINITY)
                //{
                //    rl.rlim_max = 1024;
                //}
                //for (unsigned int i=0; i < rl.rlim_max; i++)
                //{
                //    close(i);
                //}

                // close stdin stdout stderr
                close(0);
                close(1);
                close(2);

                // attach file descriptores 0,1,2 to /dev/null
                int fd0 = open("/dev/null", O_RDWR);
                int fd1 = dup(0);
                int fd2 = dup(0);
                if (fd0 !=0 || fd1 != 1 || fd2 != 2)
                {
                    ERROR("daemon unexpected file descriptors as stdin stdout stderr");
                    return false;
                }

                return true;
            }


            // sleep unit is ms
            static void Sleep(int ms)
            {
                // don't use usleep, because it's use SIGALRM may be confuse other's alarm signal.
                //usleep(ms*1000);
                struct timeval tv;
                tv.tv_sec = ms / 1000;
                tv.tv_usec = (ms % 1000) * 1000;
                ::select(0, NULL, NULL, NULL, &tv);
            }

            static unsigned long GetUptime()
            {
                struct sysinfo s_info;
                if(::sysinfo(&s_info) != 0)
                {
                    ERROR("call sysinfo get uptime fail");
                    return 0;
                }
                return s_info.uptime;
            }

            static std::string Strip(const std::string &str)
            {
                size_t pos_first = str.find_first_not_of(' ');
                size_t pos_last = str.find_last_not_of(' ');

                if (pos_first == std::string::npos ||
                        pos_last == std::string::npos)
                {
                    return std::string("");
                }
                else
                {
                    assert(pos_last >= pos_first);
                    return std::string(str, pos_first, pos_last-pos_first+1);
                }
            }

            static bool StrToInt(const std::string &str, long &nRet)
            {
                char *endptr = NULL;
                long val = ::strtol(str.c_str(), &endptr, 10);

                /* Check for various possible errors */
                if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) ||
                        (errno != 0) || 
                        (endptr == str.c_str())
                   ) 
                {
                    return false;
                }

                nRet = val;
                return true;
            }

            static unsigned int next_pow_of_2(unsigned int x)
            {
                if (!(x & (x-1)))
                {
                    //is_pow_of_2
                    return x;
                }
                x |= x>>1;
                x |= x>>2;
                x |= x>>4;
                x |= x>>8;
                x |= x>>16;
                return x+1;
            }

            static bool system_check(const char *command)
            {
                sighandler_t old_handler = signal(SIGCHLD, SIG_DFL);
                int status = ::system(command);
                INFO("%s : system return %d", command, status);
                if (-1 != status && WIFEXITED(status) && (0 == WEXITSTATUS(status)))
                {
                    signal(SIGCHLD, old_handler);
                    return true;
                }
                signal(SIGCHLD, old_handler);
                return false;
            }

            static bool system_exec(const char *command, std::string &ret)
            {
                sighandler_t old_handler = signal(SIGCHLD, SIG_DFL);
                FILE *p = ::popen(command, "r");
                if (!p)
                {
                    signal(SIGCHLD, old_handler);
                    return false;
                }

                char buffer[128];
                ::memset(buffer, 0, sizeof(buffer));
                while (!::feof(p))
                {
                    if (::fgets(buffer, 128, p) != NULL)
                        ret += buffer;
                }
                ::pclose(p);
                signal(SIGCHLD, old_handler);
                return true;
            }
    };
}

#endif
