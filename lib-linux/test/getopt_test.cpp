#include <iostream>
#include "getopt_wrap.h"
using namespace std;
using namespace lib_linux;

int main(int argc, char *argv[])
{
    GetOpt opt(argc, argv);
    opt.AddOpt('h', "help");
    opt.AddOpt('o', "", true);
    opt.AddOpt('v', "");
    
    if (opt.Parse())
    {
        for (GetOpt::iterator it=opt.begin(); it!=opt.end(); it++)
        {
            switch (it->short_opt)
            {
                case 'h':
                    cout<<"help"<<endl;
                    break;
                case 'o':
                    cout<<"output "<<it->arg<<endl;
                    break;
                case 'v':
                    cout<<"v1.0"<<endl;
                    break;
            }
        }

        const vector<string> &arg = opt.GetArgument();
        cout<<arg.size()<<endl;
        for (vector<string>::const_iterator it=arg.begin(); it!=arg.end(); it++)
        {
            cout<<*it<<endl;
        }

        return 0;
    }
    else
    {
        cerr<<"wrong options"<<endl;
        return 1;
    }
}
