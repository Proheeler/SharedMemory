#include <iostream>
#include <string.h>
#include <cassert>
#include "BaseShmm.h"
#include "ext.h"
#include "ArrayShm.h"
#include "VariantNode.h"
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

    VariantNode node(sizeof(size_t));
    NodeInfo info = node.info();
    std::cout<<info.currentShmid_<<std::endl;
    std::cout<<node.writeNodeInfo()<<std::endl;

    size_t data = 100500;
    std::cout<<node.writeData(&data)<<std::endl;
    BaseShmm test_node(info.currentShmid_);
    std::cout<<*((size_t*)(static_cast<char*>(test_node.readFromSegment(0))+sizeof(NodeInfo)))<<std::endl;
    std::cout<<((NodeInfo*)(test_node.readFromSegment(0)))->dataSize_<<std::endl;
    std::cout<<((NodeInfo*)(test_node.readFromSegment(0)))->nextShmid_<<std::endl;
    std::cout<<((NodeInfo*)(test_node.readFromSegment(0)))->prevShmid_<<std::endl;
    std::cout<<((NodeInfo*)(test_node.readFromSegment(0)))->currentShmid_<<std::endl;
    node.~VariantNode();



    Array arr1;
    arr1 = FindSegments();
    assert(arr1.size ==0);
    FreeResultOfFindSegments(arr1.pointer);
    return 0;
}

