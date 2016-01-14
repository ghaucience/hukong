#ifndef __THREAD_WRAP__
#define __THREAD_WRAP__
#include <pthread.h>
#include "semaphore_wrap.h"


namespace lib_linux
{
    class Thread
    {
        public:
            Thread(bool bAutoStart=false);
            virtual ~Thread();

            void Start();
            void Wait();
            pthread_t GetThreadID();


            // initizlize thread-specific data
            static void Initialize();
        protected:
            virtual void Run() =0;

        private:
            Thread(const Thread &);
            const Thread &operator=(const Thread &);

            static void *ThreadFuntion(void *his);

        protected:
            // thread-specific data for all threads
            // access by pthread_getspecific and pthread_setspecific
            // all thread has it's private data.
            static pthread_key_t    s_MainKey;

        private:
            // thread id
            pthread_t m_threadID;

            // set detach thread, clean up automatically when it terminates.
            // so don't join, but no way to synchronize on its completion and obtain its return value.
            pthread_attr_t m_attr;

            //Semaphore m_sem;
            // wait for thread to exit
            Semaphore m_semWait;

            // is thread running
            bool m_bRunning;
    };
}

#endif
