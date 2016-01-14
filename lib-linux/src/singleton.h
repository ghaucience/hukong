//
//  单件模式类(不是线程安全)
//  author: bobo
//  
#ifndef _SINGLETON_H_
#define _SINGLETON_H_

template<class T>
// TODO 如果类需要派生才能初始化
class Singleton
{
    public:
        static T *Instance()
        {
            static T _instance;
            return &_instance;
        }

    protected:
        Singleton()
        {}

        Singleton(const Singleton &);
        Singleton& operator = (const Singleton<T> &);
};

#endif
