#include "ext.h"



extern "C" BaseShmm *BaseShmm_Create(int shmid)
{
    return new BaseShmm(shmid);
}

extern "C" void* LockAndRead(BaseShmm* pFoo,int64_t frameNum)
{
    return pFoo->readFromSegment(frameNum);
}
extern "C" void Unlock(BaseShmm* pFoo,int64_t frameNum) {
    return pFoo->unlock(frameNum);
}
extern "C" bool Write(BaseShmm *pFoo, int64_t frameNum, void *data)
{
    return pFoo->writeToSegment(frameNum,data);
}

extern "C" void BaseShmm_Delete(BaseShmm* pFoo) {
    delete pFoo;
}


extern "C" int Create(int64_t MemorySize, int64_t ChunkSize)
{
    return BaseShmm::createSegment(MemorySize,ChunkSize);
}

extern "C" int64_t GetChunkSize(BaseShmm *pFoo)
{
    return pFoo->getBlockSize();
}

extern "C" void Delete(int shmid)
{
    BaseShmm::deleteSegment(shmid);
}

extern "C" int64_t GetMemSize(BaseShmm *pFoo)
{
    return pFoo->getMemorySize();
}

extern "C" bool checkShmid(int shmid)
{
    return BaseShmm::checkShmid(shmid);
}

extern "C" int64_t numOfBlocks(BaseShmm *pFoo)
{
    return pFoo->getMaxSize();
}

my_array FindSegments()
{
    return BaseShmm::findSegments();
}

void FreeResultOfFindSegments(int *pointer)
{
    delete[] pointer;
}
