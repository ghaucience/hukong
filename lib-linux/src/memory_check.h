#ifndef __MEMORY_CHECK__
#define __MEMORY_CHECK__
// check memory leak and overflow.

#ifdef _DEBUG
#include <new>
#include "Mutex.h"
#include "queue.h"

namespace lib_linux
{
    enum
    {
        // max filename size of __FILE__
        MAX_FILENAMESIZE = 48
    };

    // tag define
    struct TagElem
    {
        char fileName[MAX_FILENAMESIZE];
        int line;
        void *pMemory;
        int size;
        SLIST_ENTRY(TagElem) _entry;
    };

    // list head type def
    SLIST_HEAD(TagHeadType, TagElem);

    class MemoryCheck
    {
        public:
            static void *DebugNew(size_t size, const char *file, int nLine);
            static void DebugDelete(void *mem);

            static void ValidateMemory(void *mem);
            static int ValidateMemoryAll();

        private:
            // mutex to solve mutilthread call new.
            static Mutex s_mutex;
            // tag list
            static TagHeadType s_tagList;
    };
}

// global namespace
inline void *operator new(std::size_t s, const char *file, int nLine)
{
    return lib_linux::MemoryCheck::DebugNew(s, file, nLine);
}

inline void *operator new[](std::size_t s, const char *file, int nLine)
{
    return lib_linux::MemoryCheck::DebugNew(s, file, nLine);
}

inline void operator delete(void *mem)
{
    lib_linux::MemoryCheck::DebugDelete(mem);
}

inline void operator delete[](void *mem)
{
    lib_linux::MemoryCheck::DebugDelete(mem);
}

// redefine new
#define new new(__FILE__, __LINE__)

#endif //_DEBUG
#endif //__MEMORY_CHECK__
