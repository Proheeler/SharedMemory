#include "ext.h"

extern "C" BaseShmm *BaseShmm_Create(int shmid)
{
    return new BaseShmm(shmid);
}

extern "C" void* LockAndRead(BaseShmm* pFoo,size_t frameNum)
{
    return pFoo->readFromSegment(frameNum);
}
extern "C" void Unlock(BaseShmm* pFoo,size_t frameNum) {
    return pFoo->unlock(frameNum);
}
extern "C" bool Write(BaseShmm *pFoo, size_t frameNum, void *data)
{
    return pFoo->writeToSegment(frameNum,data);
}

extern "C" void BaseShmm_Delete(BaseShmm* pFoo) {
    delete pFoo;
}


extern "C" int Create(size_t MemorySize, size_t ChunkSize)
{
    return BaseShmm::createSegment(MemorySize,ChunkSize);
}

extern "C" size_t GetChunkSize(BaseShmm *pFoo)
{
    return pFoo->getBlockSize();
}

extern "C" void Delete(int shmid)
{
    BaseShmm::deleteSegment(shmid);
}

extern "C" size_t GetMemSize(BaseShmm *pFoo)
{
    return pFoo->getMemorySize();
}

extern "C" bool checkShmid(int shmid)
{
    return BaseShmm::checkShmid(shmid);
}

extern "C" size_t numOfBlocks(BaseShmm *pFoo)
{
    return pFoo->getMaxSize();
}

extern "C" Array FindSegments()
{
    return BaseShmm::findSegments();
}

extern "C" void FreeResultOfFindSegments(int *pointer)
{
    delete[] pointer;
}

extern "C" void ReinitSharedMemoryOnNewChunSize(BaseShmm *pFoo, size_t ChunkSize)
{
    pFoo->reinitMemory(ChunkSize);
}
