#ifndef __SEMAPHORE_WRAP_H__
#define __SEMAPHORE_WRAP_H__
#include <semaphore.h>
#include "lib_linux_config.h"

namespace lib_linux
{
    class Semaphore
    {
        public:
            Semaphore(unsigned int unStartVal=0)
            {
                ::sem_init(&m_sem, 0, unStartVal);
            }

            ~Semaphore()
            {
                ::sem_destroy(&m_sem);
            }

            void Wait()
            {
                ::sem_wait(&m_sem);
            }

            void Post()
            {
                ::sem_post(&m_sem);
            }

            bool TryWait()
            {
                if (::sem_trywait(&m_sem) == 0)
                {
                    return true;
                }
                else
                {
                    if (errno != EAGAIN)
                    {
                        ERROR("Semaphore sem_trywait error!");
                    }

                    return false;
                }
            }

        private:
            Semaphore(const Semaphore &);
            const Semaphore &operator =(const Semaphore &);

        private:
            sem_t m_sem;
    };
}

#endif
