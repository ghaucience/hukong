#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <pthread.h>
#include "Mutex.h"
#include <sys/time.h>
#include "lib_linux_config.h"

namespace lib_linux
{
    // condition like auto event object on windows system, so can ocu
    class Condition
    {
        public:
            Condition()
            {
                ::pthread_condattr_init(&m_attr);                                                  
                ::pthread_condattr_setclock(&m_attr, CLOCK_MONOTONIC);                             
                ::pthread_cond_init(&m_cond, &m_attr);
            }

            ~Condition()
            {
                ::pthread_cond_destroy(&m_cond);
                ::pthread_condattr_destroy(&m_attr);
            }

            // wait block condition and unlock mutex. so mutex and conditon are this:
            // condition:
            //                signal
            //                  _
            // _________________ ______________
            //
            // mutex:
            // _______ call wait ______________
            //        ___________
            // so if you wait the mutex, only call Wait when mutex is unlock, and don't miss signal of condiction.
            bool Wait(Mutex &mutex)
            {
                return ::pthread_cond_wait(&m_cond, mutex.GetMutex()) == 0;
            }

            // nMilSec millseconds
            bool Wait(Mutex &mutex, unsigned int unMilSec)
            {
                struct timespec ts;
                ::clock_gettime(CLOCK_MONOTONIC, &ts);

                unsigned int sec, usec;
                sec = unMilSec / 1000;
                usec = (unMilSec % 1000) * 1000;

                ts.tv_sec += sec;
                ts.tv_nsec += (usec*1000);

                if(ts.tv_nsec > 999999999)
                {
                    ts.tv_sec++;
                    ts.tv_nsec -= 1000000000;
                }

                return ::pthread_cond_timedwait(&m_cond, mutex.GetMutex(), &ts) == 0;
            }

            void Signal()
            {
                ::pthread_cond_signal(&m_cond);
            }

            void SignalAll()
            {
                ::pthread_cond_broadcast(&m_cond);
            }

        private:
            pthread_condattr_t m_attr;
            pthread_cond_t     m_cond;
    };
}

#endif
