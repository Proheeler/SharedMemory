#include <iostream>
#include <string.h>
#include <cassert>
#include "BaseShmm.h"
#include "ext.h"
#include "ArrayShm.h"
#include "VariantNode.h"
#include "LinkedVariantList.h"
#include "SharedVector.h"
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

    size_t data = 100500;
    std::cout<<node.writeData(&data)<<std::endl;
    BaseShmm test_node(info.currentShmid_);
    std::cout<<*((size_t*)(static_cast<char*>(test_node.readFromSegment(0))+sizeof(NodeInfo)))<<std::endl;
    std::cout<<((NodeInfo*)(test_node.readFromSegment(0)))->dataSize_<<std::endl;
    std::cout<<((NodeInfo*)(test_node.readFromSegment(0)))->nextShmid_<<std::endl;
    std::cout<<((NodeInfo*)(test_node.readFromSegment(0)))->prevShmid_<<std::endl;
    std::cout<<((NodeInfo*)(test_node.readFromSegment(0)))->currentShmid_<<std::endl;

    auto node_copy = VariantNode::createFromExisting(info.currentShmid_);
    assert(node_copy.info().currentShmid_ == info.currentShmid_ );
    assert(node_copy.info().dataSize_ == info.dataSize_ );


    VariantNode node2(sizeof(size_t));
    NodeInfo info2 = node2.info();
    info2.prevShmid_ = node.info().currentShmid_;
    node2.setInfo(info2);
    std::cout<<info2.currentShmid_<<std::endl;
    size_t data2 = 100600;
    std::cout<<node2.writeData(&data2)<<std::endl;
    info.nextShmid_ = info2.currentShmid_;
    node.setInfo(info);
    SharedLinkedVariantList list(info.currentShmid_);
    VariantNode node3(sizeof(size_t));
    std::cout<<node3.writeData(&data2)<<std::endl;
    VariantNode node4(sizeof(size_t));
    size_t data3 = 100700;
    std::cout<<node4.writeData(&data3)<<std::endl;
    list.insert(10,std::move(node3));
    list.pushback(std::move(node4));
    void* nodeData = list.at(0).getData();
    NodeInfo info_test =*(NodeInfo*)nodeData;
    assert(info_test == info);
    size_t datafromnode = *(size_t*)((char*)nodeData+sizeof(NodeInfo));
    assert(datafromnode == data);

    list.remove(0);
    list.remove(2);

    SharedVector<int> vect;
    vect.push_back(10);
    std::cout<<"////////////////////////"<<std::endl;
    vect.printData();
    vect.push_back(20);

    vect.push_back(30);
    //
    std::cout<<"////////////////////////"<<std::endl;
    vect.printData();
    Array arr1;
    arr1 = FindSegments();
    assert(arr1.size >0);
    FreeResultOfFindSegments(arr1.pointer);
    return 0;
}

