#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "thread_wrap.h"
#include "jobqueue.h"
using namespace std;
using namespace lib_linux;

static JobQueue<int> g_queue;

class PushThread: public Thread
{
    public:
        PushThread()
        :Thread(true)
        { }

    protected:
        void Run()
        {
            cout<<"PushThread"<<endl;
            while (true)
            {
                int i = 1;
                sleep(1);
                g_queue.Push(i);
            }
        }
};

class PopThread: public Thread
{
    public:
        PopThread()
        :Thread(true)
        {
        }

    protected:
        void Run()
        {
            cout<<"PopThread"<<endl;
            while (true)
            {
                int nValue;
                DEBUG("enter jobqueue waitpop");
                if (g_queue.WaitPop(500, nValue))
                {
                    cout<<nValue<<" size:"<<g_queue.Size()<<endl;
                }
                DEBUG("exit jobqueue waitpop");
            }
        }
};

int main(int argc, char **argv)
{
    PushThread threadPush;
    PopThread threadPop;

    threadPush.Wait();
    threadPop.Wait();
    return 0;
}
