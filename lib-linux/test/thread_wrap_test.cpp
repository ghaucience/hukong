#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "Mutex.h"
#include "thread_wrap.h"
using namespace std;
using namespace lib_linux;

class MyThread: public Thread
{
    public:
        MyThread(int num, Mutex &mutex)
            :Thread(true),
            m_num(num),
            m_mutex(mutex)
        {
        }

    protected:
        void Run()
        {
            while (true)
            {
                sleep(m_num);
                AutoLock lock(m_mutex);
                cout<<m_num<<endl;
            }
        }

    private:
        int m_num;
        Mutex &m_mutex;
};

int main(int argc, char **argv)
{
    Mutex mutex;

    MyThread thread1(1, mutex);
    MyThread thread2(2, mutex);
    MyThread thread3(3, mutex);
    MyThread thread4(4, mutex);
    MyThread thread5(5, mutex);
    MyThread thread6(6, mutex);
    MyThread thread7(7, mutex);

    thread1.Wait();
    thread2.Wait();
    thread3.Wait();
    thread4.Wait();
    thread5.Wait();
    thread6.Wait();
    thread7.Wait();
    return 0;
}
