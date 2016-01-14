#ifndef __JOBQUEUE_H__
#define __JOBQUEUE_H__

#include <deque>
#include "condition.h"
#include "Mutex.h"
#include "lib_linux_config.h"

namespace lib_linux
{
    // job queue for thread communication.
    template <class T>
        class JobQueue
        {
            public:
                JobQueue()
                {}

                ~JobQueue()
                {
                    //DEBUG("JobQueue destory");
                    //assert(m_queue.size() == 0);
                }

                int Size()
                {
                    AutoLock lock(m_mutex);
                    return m_queue.size();
                }

                void Clear()
                {
                    AutoLock lock(m_mutex);
                    m_queue.clear();
                }

                void Push(T &elem)
                {
                    //DEBUG("JobQueue Push");
                    //{
                    // only enqueue in WaitPop called. 
                    // see condition.h explain it.
                    AutoLock lock(m_mutex);
                    m_queue.push_back(elem);
                    //}

                    // this only active one thread.
                    m_condition.Signal();
                }

                bool WaitPop(unsigned int unMilSecs, T &ret)
                {
                    AutoLock lock(m_mutex);
                    if (m_queue.empty())
                    {
                        // call this will unlock mutex, so thread can push only this time.
                        if (!m_condition.Wait(m_mutex, unMilSecs))
                        {
                            return false;
                        }
                    }

                    // check again
                    if (!m_queue.empty())
                    {
                        ret = m_queue.front();
                        m_queue.pop_front();
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }

            private:
                Mutex m_mutex;
                std::deque<T> m_queue;
                Condition m_condition;
        };
}

#endif
