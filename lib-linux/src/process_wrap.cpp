#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <memory>

#include <sys/wait.h>

#include "process_wrap.h"
#include "lib_linux_config.h"

namespace lib_linux
{
    Process::Process(const char *program, const char *arg)
        :m_childPID(0)
    {
        if (!CreateProcess(program, arg))
        {
            throw "CreateProcess fail";
        }
    }

    Process::~Process()
    {
    }

    pid_t Process::GetPID()
    {
        assert(m_childPID != 0 && "No child process create");
        return m_childPID;
    }

    pid_t Process::GetPPID()
    {
        return ::getpid();
    }

    bool Process::SigactionSet(int signum, SA_HANDLER handler)
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = handler;
        return ::sigaction(signum, &sa, NULL) == 0;
    }

    bool Process::SigactionIgnore(int signum)
    {
        return SigactionSet(signum, SIG_IGN);
    }

    bool Process::CreateProcess(const char *file, const char *arg)
    {
        pid_t child_pid = ::fork();
        if (child_pid == -1)
        {
            return false;
        }

        if (child_pid ==0 )
        {
            std::vector<std::string> vec_str;
            vec_str.push_back(file);

            std::stringstream stream(arg);
            std::copy(std::istream_iterator<std::string>(stream),
                    std::istream_iterator<std::string>(), 
                    std::back_inserter(vec_str));

            std::auto_ptr<const char*> arg(new const char*[vec_str.size()+1]);
            ::memset(arg.get(), 0, sizeof(const char*)*(vec_str.size()+1));
            int index=0;
            for (std::vector<std::string>::iterator it=vec_str.begin(); it!=vec_str.end(); it++)
            {
                DEBUG("%s\n", (*it).c_str());
                arg.get()[index++] = (*it).c_str();
            }

            //for (int i=0; i<(vec_str.size()+1); i++)
            //{
            //    TRACE("%d\n", (int)(arg.get()[i]));
            //}

            // execute program, searching for it in the path.
            ::execvp(file, (char* const*)arg.get());

            // execvp function return only if an error occours.
            ::fprintf(stderr, "an error occurred in execvp: %s", strerror(errno));
            ::abort();
        }
        else
        {
            m_childPID = child_pid;
        }

        return true;
    }

    void Process::Kill()
    {
        assert(m_childPID != 0 && "No child process create");
        ::kill(m_childPID, SIGTERM);
    }

    bool Process::Wait()
    {
        assert(m_childPID != 0 && "No child process create");
        int status;
        ::waitpid(m_childPID, &status, 0);
        return WIFEXITED(status);
    }
}
