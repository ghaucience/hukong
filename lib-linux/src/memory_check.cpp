#ifdef _DEBUG
#include "memory_check.h"
#include <cstdlib>
#include <cstring>
#include "lib_linux_config.h"

namespace lib_linux
{
    // static variable
    Mutex MemoryCheck::s_mutex;
    TagHeadType MemoryCheck::s_tagList = SLIST_HEAD_INITIALIZER(s_tagList);

    void *MemoryCheck::DebugNew(size_t size, const char *file, int nLine)
    {
        AutoLock lock(s_mutex);

        // mark the beginning and the end with the line number.
        int nTotalSize = size + sizeof(TagElem) + 2*(sizeof(nLine));

        // memory map: [TagElem] [nLine] [real new memory] [nLine]
        char *pAlloc = (char *)malloc(nTotalSize);
        //mark the block with an easily identifiable pattern
        memset(pAlloc, 0xCD, nTotalSize);
        memcpy(pAlloc + sizeof(TagElem), &nLine, sizeof(nLine));
        memcpy((pAlloc + nTotalSize) - sizeof(nLine), &nLine, sizeof(nLine));

        TagElem *pTag = (TagElem *)pAlloc;
        strncpy(pTag->fileName, file, MAX_FILENAMESIZE);
        pTag->fileName[MAX_FILENAMESIZE-1] = '\0';
        pTag->line = nLine;
        pTag->size = size;
        pTag->pMemory = pAlloc + sizeof(TagElem) + sizeof(nLine);

        SLIST_INSERT_HEAD(&s_tagList, pTag, _entry);
        return pTag->pMemory;
    }

    void MemoryCheck::DebugDelete(void *mem)
    {
        AutoLock lock(s_mutex);
        char *pAlloc = (char *)mem - sizeof(int) - sizeof(TagElem);
        TagElem *pTag = (TagElem *)pAlloc;

        // check wild pointer
        TagElem *pTemp = NULL;
        SLIST_FOREACH(pTemp, &s_tagList, _entry)
        {
            if (pTemp == pTag)
            {
                break;
            }
        }

        if (pTemp == NULL)
        {
            assert(0 && "Error: delete wild pointer");
            abort();
        }

        // check overflow
        ValidateMemory(pAlloc);

        // remove entry from taglist
        SLIST_REMOVE(&s_tagList, pTag, TagElem, _entry);

        int nTotalSize = sizeof(TagElem) + 2*sizeof(pTag->line) + pTag->size;
        // delete our memory block
        ::memset(pAlloc, 0xFE, nTotalSize);
        free(pAlloc);
    }


    void MemoryCheck::ValidateMemory(void *mem)
    {
        char *pAlloc = (char *)mem;
        TagElem *pTag = (TagElem *)pAlloc;
        int nTagLine1 = *(int *)(pAlloc + sizeof(TagElem));
        int nTagLine2 = *(int *)(pAlloc + sizeof(TagElem) + sizeof(pTag->line) + pTag->size);
        if (nTagLine1 != pTag->line || nTagLine2 != pTag->line)
        {
            assert(false && "Error: memory overflow");
            abort();
        }
    }

    int MemoryCheck::ValidateMemoryAll()
    {
        int size = 0;
        TagElem *pTemp = NULL;
        SLIST_FOREACH(pTemp, &s_tagList, _entry)
        {
            char *pAlloc = (char *)pTemp;
            ValidateMemory(pAlloc);
            size++;
        }
        return size;
    }
}
#endif
