#pragma once
#include "shmm_v1.h"

extern "C" BaseShmm* BaseShmm_Create(int shmid);
extern "C" int Create(int64_t MemorySize, int64_t ChunkSize);
extern "C" void* LockAndRead(BaseShmm* pFoo,int64_t frameNum);
extern "C" int64_t GetChunkSize(BaseShmm* pFoo);
extern "C" int64_t GetMemSize(BaseShmm* pFoo);
extern "C" void Unlock(BaseShmm* pFoo,int64_t frameNum);
extern "C" void BaseShmm_Delete(BaseShmm* pFoo);
extern "C" bool Write(BaseShmm* pFoo,int64_t frameNum,void* data);
extern "C" void Delete(int shmid);
extern "C" bool checkShmid(int shmid);
extern "C" int64_t numOfBlocks(BaseShmm* pFoo);
extern "C" my_array FindSegments();
extern "C" void FreeResultOfFindSegments(int* pointer);

