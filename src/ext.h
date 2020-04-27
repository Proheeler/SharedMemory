#pragma once
#include "BaseShmm.h"



extern "C" BaseShmm* BaseShmm_Create(int shmid);
extern "C" int Create(size_t MemorySize, size_t ChunkSize);
extern "C" void* LockAndRead(BaseShmm* pFoo,size_t frameNum);
extern "C" size_t GetChunkSize(BaseShmm* pFoo);
extern "C" size_t GetMemSize(BaseShmm* pFoo);
extern "C" void Unlock(BaseShmm* pFoo,size_t frameNum);
extern "C" void BaseShmm_Delete(BaseShmm* pFoo);
extern "C" bool Write(BaseShmm* pFoo,size_t frameNum,void* data);
extern "C" void Delete(int shmid);
extern "C" bool checkShmid(int shmid);
extern "C" size_t numOfBlocks(BaseShmm* pFoo);
extern "C" Array FindSegments();
extern "C" void FreeResultOfFindSegments(int* pointer);
extern "C" void ReinitSharedMemoryOnNewChunSize(BaseShmm* pFoo,size_t ChunkSize);
