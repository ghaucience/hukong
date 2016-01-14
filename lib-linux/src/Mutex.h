#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <pthread.h>

namespace lib_linux
{
    class Mutex
    {
        public:
            Mutex()
            {
                ::pthread_mutex_init(&m_mutex, NULL);
            }

            ~Mutex()
            {
                ::pthread_mutex_destroy(&m_mutex);

            }

            void Lock()
            {
                ::pthread_mutex_lock(&m_mutex);
            }

            bool TryLock()
            {
                return ::pthread_mutex_trylock(&m_mutex) == 0;
            }

            void UnLock()
            {
                ::pthread_mutex_unlock(&m_mutex);
            }

            pthread_mutex_t *GetMutex()
            {
                return &m_mutex;
            }

        private:
            Mutex(const Mutex &);
            const Mutex &operator=(const Mutex &);
        private:
            pthread_mutex_t m_mutex;
    };

    // Auto Lock 
    class AutoLock
    {
        public:
            AutoLock(Mutex &mutex)
                :m_mutex(mutex)
            {
                m_mutex.Lock();
            }

            ~AutoLock()
            {
                m_mutex.UnLock();
            }

        private:
            Mutex &m_mutex;
    };
}

#endif
