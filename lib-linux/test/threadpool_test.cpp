#include <stdlib.h>
#include <iostream>
#include "threadpool.h"
using namespace std;
using namespace lib_linux;

void Print(void *pvoid)
{
    cout<<"hello"<<endl;
}

int main(int argc, char **argv)
{
    ThreadPool pool;
    pool.Start(5);

    pool.Run(std::ptr_fun(Print));
    pool.Run(std::ptr_fun(Print));
    pool.Run(std::ptr_fun(Print));
    pool.Run(std::ptr_fun(Print));
    pool.Run(std::ptr_fun(Print));
    pool.Run(std::ptr_fun(Print));
    pool.Run(std::ptr_fun(Print));
    sleep(2);
    pool.Stop();
    return 0;
}
