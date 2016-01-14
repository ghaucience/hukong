#include "thread_wrap.h"
#include "utility.h"
#include "lib_linux_config.h"

namespace lib_linux
{
    pthread_key_t Thread::s_MainKey;

    void Thread::Initialize()
    {
        ::pthread_key_create(&Thread::s_MainKey, NULL);
    }

    Thread::Thread(bool bAutoStart)
        :m_threadID(-1), 
        m_bRunning(false)
    {
        ::pthread_attr_init(&m_attr);

        if (bAutoStart)
        {
            Start();
        }
    }

    Thread::~Thread()
    {
        //DEBUG("call thread wait");
        Wait();
        //DEBUG("call thread wait ok");
        ::pthread_attr_destroy(&m_attr);
    }

    void *Thread::ThreadFuntion(void *his)
    {
        // TODO change to sync cancel mode
        ::pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

        // Sleep here to avid crash. 
        // because dervied thread class not constructor to setup vtable,
        // and call pure virtual function.
        Utility::Sleep(5);

        //DEBUG("ready call thread run");
        Thread *h=(Thread *)his;
        //h->m_sem.Wait();
        h->Run();
        h->m_semWait.Post();
        //DEBUG("call thread run ok");
        return NULL;
    }

    void Thread::Start()
    {
        if (!m_bRunning)
        {
            // set detached thread
            ::pthread_attr_setdetachstate(&m_attr, PTHREAD_CREATE_DETACHED);
            if (::pthread_create(&m_threadID, &m_attr, &ThreadFuntion, this) != 0)
            {
                ERROR("pthread_create create fail");
                ::abort();
            }

            //m_sem.Post();
            m_bRunning = true;
        }
        else
        {
            WARNING("Thread: Duplication call Start");
        }
    }

    void Thread::Wait()
    {
        if (m_bRunning)
        {
            // pthread_join only used to undetached thread
            //::pthread_join(m_threadID, NULL);
            m_semWait.Wait();
            m_bRunning = false;
            m_threadID = -1;
        }
    }

    pthread_t Thread::GetThreadID()
    {
        return m_threadID;
    }
}
