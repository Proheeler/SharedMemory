#include <iostream>
#include <string.h>
#include <cassert>
#include "BaseShmm.h"
#include "ext.h"
#include "ArrayShm.h"
int main()
{
    int shmid =Create(sizeof(int)*10+100,4);
    BaseShmm* shmm = BaseShmm_Create(shmid);
    int a[]={1,2,3,4,5,6,7,8,9,0};
    for(int i=0;i<10;++i)
    {
        Write(shmm,i,&a[i]);
    }
    std::cout<<shmid<<std::endl;
    for(auto i = shmm->begin();i!= shmm->end();++i)
    {
        std::cout<<*(static_cast<int*>(*i))<<std::endl;
    }
    for(auto i:*shmm)
    {
        std::cout<<*(static_cast<int*>(i))<<std::endl;
    }

    std::cout<<shmm->getMaxSize()<<std::endl;
    std::cout<<shmm->getBlockSize()<<"\t"<<shmm->getMemorySize()<<std::endl;
    shmm->reinitMemory(16);
    std::cout<<shmm->getMaxSize()<<std::endl;
    std::cout<<shmm->getBlockSize()<<"\t"<<shmm->getMemorySize()<<std::endl;

    Array arr;
    arr = FindSegments();
    for(int i=0;i<arr.size;++i)
    {
        std::cout<<i<<" "<<*(arr.pointer+i)<<std::endl;
        Delete(*(arr.pointer+i));
    }
    FreeResultOfFindSegments(arr.pointer);
    ArrayShm<char> arrShm(10);
    std::cout<<"array size: "<<arrShm.size()<<std::endl;
    *arrShm.at(0)='u';
    printf("array at 0 : %c\n",*arrShm.at(0));
    char ac[]="1234567890";
    for(size_t i=0;i<arrShm.size();++i)
    {
       arrShm.write(i,&ac[i],true);
       assert(ac[i] == *arrShm.at(i));
       std::cout<<*arrShm.at(i)<<std::endl;
    }
    BaseShmm_Delete(shmm);
    BaseShmm::deleteSegment(shmid);
    arrShm.~ArrayShm();
    Array arr1;
    arr1 = FindSegments();
    assert(arr1.size ==0);
    FreeResultOfFindSegments(arr1.pointer);
    return 0;
}

