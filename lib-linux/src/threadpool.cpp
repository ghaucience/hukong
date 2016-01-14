#include "threadpool.h"
#include "lib_linux_config.h"

namespace lib_linux
{
    ThreadPool::ThreadTask::ThreadTask(ThreadPool &pool)
        :m_pool(pool)
    {
    }

    void ThreadPool::ThreadTask::Run()
    {
        try
        {
            while (m_pool.m_running)
            {
                ThreadPool::Task task;
                if (m_pool.Take(task))
                {
                    task(NULL);
                }
            }
        }
        catch (...)
        {
            ERROR("exception caugth in threadpool");
            ::abort();
        }
    }


    ThreadPool::ThreadPool()
        :m_running(false)
    {
    }

    ThreadPool::~ThreadPool()
    {
        if (m_running)
        {
            Stop();
        }
    }

    void ThreadPool::Start(int numThreads)
    {
        assert(numThreads > 0);
        assert(m_threads.empty());
        m_running = true;
        for (int i=0; i<numThreads; i++)
        {
            ThreadTask *pThread = new ThreadTask(*this);
            m_threads.push_back(pThread);
            pThread->Start();
        }
    }

    void ThreadPool::Stop()
    {
        m_running = false;
        // wait for threads exit
        for (std::vector<ThreadTask *>::iterator it=m_threads.begin(); it!=m_threads.end(); it++)
        {
            (*it)->Wait();
            delete (*it);
        }
        m_threads.clear();
    }

    void ThreadPool::Run(Task f)
    {
        assert(m_running && "threadpool is not running");
        m_taskQueue.Push(f);
    }

    bool ThreadPool::Take(Task &task)
    {
        // wait 0.5 second
        return m_taskQueue.WaitPop(500, task);
    }
}
