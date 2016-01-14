#ifndef __GETOPT_WRAP__
#define __GETOPT_WRAP__

#include <string>
#include <vector>
#include <map>

namespace lib_linux
{
    class GetOpt
    {
        public:
            // options element
            struct Opt
            {
                char short_opt;
                std::string long_opt;
                std::string arg;
                bool has_arg;
            };

        public:
            typedef std::vector<Opt>::const_iterator iterator;

            iterator begin() const;
            iterator end() const;

        public:
            GetOpt(int argc, char **argv);
            ~GetOpt();

            // initial the options
            void AddOpt(char short_opt, const char *long_opt, bool has_arg=false);

            bool Parse();

            // return the nonoption argument
            const std::vector<std::string> &GetArgument();
        private:
            int m_argc;
            char **m_argv;

            std::map<char, Opt> m_mapOpts;
            // finaly result
            std::vector<Opt> m_opts;

            // nonoptions argument
            std::vector<std::string> m_argument;
    };
}

#endif
