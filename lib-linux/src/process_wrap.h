#ifndef __PROCESS_WRAP__
#define __PROCESS_WRAP__

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

namespace lib_linux
{
    class Process
    {
        public:
            // Signal handler function
            typedef void (*SA_HANDLER)(int);

        public:
            Process(const char *program, const char *arg);
            ~Process();

            // get child process pid
            pid_t GetPID();
            // get current process pid
            pid_t GetPPID();

            // Signal process
            static bool SigactionSet(int signum, SA_HANDLER handler);
            static bool SigactionIgnore(int signum);

            void Kill();

            bool Wait();

        protected:
            bool CreateProcess(const char *file, const char *arg);

        private:
            pid_t m_childPID;
    };
}

#endif
