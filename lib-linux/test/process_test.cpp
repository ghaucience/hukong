#include <stdlib.h>
#include <iostream>
#include "process_wrap.h"
using namespace std;
using namespace lib_linux;

void Handler(int signum)
{
    cout<<"Hi, in Handler "<<signum<<endl;
}

int main(int argc, char **argv)
{
    Process proc("firefox", "");
    cout<<"PID: "<<proc.GetPID()<<endl;
    //Process::SigactionSet(SIGTERM, Handler);
    
    // make zombie process 
    proc.Kill();

    ::sleep(20);

    return 0;
}
