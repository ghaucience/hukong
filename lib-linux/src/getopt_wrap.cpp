#include <memory>
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include "getopt_wrap.h"
#include "lib_linux_config.h"

// option argument
extern char *optarg;
// first nonoption argument
extern int optind;

namespace lib_linux
{
    GetOpt::GetOpt(int argc, char **argv)
        :m_argc(argc), m_argv(argv)
    {
    }

    GetOpt::~GetOpt()
    {
    }

    GetOpt::iterator GetOpt::begin() const
    {
        return m_opts.begin();
    }

    GetOpt::iterator GetOpt::end() const
    {
        return m_opts.end();
    }

    void GetOpt::AddOpt(char short_opt, const char *long_opt, bool has_arg)
    {
        Opt opt;
        opt.short_opt = short_opt;
        opt.long_opt = long_opt;
        opt.arg = "";
        opt.has_arg = has_arg;
        assert(m_mapOpts.count(short_opt) == 0);
        m_mapOpts[short_opt] = opt;
    }

    bool GetOpt::Parse()
    {
        assert(m_mapOpts.size() > 0 && m_opts.size() == 0);

        std::string str_short_options;
        std::auto_ptr<struct option> long_options(new option[m_mapOpts.size() + 1]);
        ::memset(long_options.get(), 0, sizeof(struct option)*(m_mapOpts.size()+1));

        int index=0;
        for (std::map<char, Opt>::iterator it=m_mapOpts.begin(); it!=m_mapOpts.end(); it++)
        {
            str_short_options += it->second.short_opt;
            if (it->second.has_arg)
            {
                str_short_options += ":";
            }

            long_options.get()[index].name = it->second.long_opt.c_str();
            long_options.get()[index].has_arg = it->second.has_arg;
            long_options.get()[index].flag = NULL;
            long_options.get()[index].val = it->second.short_opt;
            index++;
        }

        int next_option = -1;
        do
        {
            next_option = getopt_long(m_argc, m_argv, str_short_options.c_str(),
                    long_options.get(), NULL);

            //TRACE("%s %d\n", str_short_options.c_str(), next_option);
            if (next_option == -1)
            {
                // Done with options.
            }
            else if (next_option == '?' || m_mapOpts.count(next_option) == 0)
            {
                // invalid option
                return false;
            }
            else
            {
                Opt opt = m_mapOpts[next_option];
                if (opt.has_arg)
                {
                    opt.arg = ::optarg;
                }

                m_opts.push_back(opt);
            }
        }while(next_option != -1);

        // nonoption argument
        for (int i=::optind; i<m_argc; i++)
        {
            m_argument.push_back(m_argv[i]);
        }

        return true;
    }

    const std::vector<std::string> &GetOpt::GetArgument()
    {
        return m_argument;
    }
}
