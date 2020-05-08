#ifndef SHAREDVECTOR_H
#define SHAREDVECTOR_H
#include "BaseShmm.h"
#include <memory>
#include <iostream>
#include <cassert>
#include <stdexcept>
typedef struct
{
    int dataBlockShmid;
    int currentUsage;
}DataBlockInfo;


template< typename T>
class SharedVector
{
public:
    SharedVector(){
        int shmidForDataBlock = BaseShmm::createSegment((5*(sizeof(T)+sizeof(std::atomic<int>*)))+1+2*sizeof(size_t),sizeof(T));
        dataBlock_ = std::make_unique<BaseShmm>(shmidForDataBlock);
        int shmidForControlBlock = BaseShmm::createSegment((sizeof(DataBlockInfo)+sizeof(std::atomic<int>*))+1+2*sizeof(size_t),sizeof(DataBlockInfo));
        controlBlock_ = std::make_unique<BaseShmm>(shmidForControlBlock);
        DataBlockInfo initialInfo = {shmidForDataBlock,0};
        controlBlock_->writeToSegment(0,&initialInfo);
    };
    SharedVector(int shmidForControlBlock)
    {
        controlBlock_ = std::make_unique<BaseShmm>(shmidForControlBlock);
        auto currentInfo = (DataBlockInfo*)(controlBlock_->readFromSegment(0));
        controlBlock_->unlock(0);
        dataBlock_ = std::make_unique<BaseShmm>(currentInfo->dataBlockShmid);
    }
    ~SharedVector(){
        BaseShmm::deleteSegment(controlBlock_->getShmid());
        BaseShmm::deleteSegment(dataBlock_->getShmid());
        controlBlock_.~unique_ptr();
        dataBlock_.~unique_ptr();
    }
    void push_back(T value)
    {
        auto currentInfo = (DataBlockInfo*)(controlBlock_->readFromSegment(0));
        if(currentInfo->currentUsage < (int)((dataBlock_->getMaxSize()-2)))
        {
            dataBlock_->writeToSegment(currentInfo->currentUsage,&value);
            DataBlockInfo newInfo = {currentInfo->dataBlockShmid,currentInfo->currentUsage+1};
            controlBlock_->unlock(0);
            controlBlock_->writeToSegment(0,&newInfo);
        }
        else
        {
            int shmidForDataBlock = BaseShmm::createSegment((2*dataBlock_->getMaxSize()*(sizeof(T)+sizeof(std::atomic<int>*)))+1+2*sizeof(size_t),sizeof(T));
            copyToNewDataBlock(shmidForDataBlock,currentInfo);
            dataBlock_->writeToSegment(currentInfo->currentUsage,&value);
            DataBlockInfo newInfo = {shmidForDataBlock,currentInfo->currentUsage+1};
            controlBlock_->unlock(0);
            controlBlock_->writeToSegment(0,&newInfo);
        }
    }
    T pop_back()
    {
        auto currentInfo = (DataBlockInfo*)(controlBlock_->readFromSegment(0));
        if(currentInfo->currentUsage > 0)
        {
            T data = *((T*)dataBlock_->readFromSegment(currentInfo->currentUsage-1));
            dataBlock_->unlock(currentInfo->currentUsage);
            DataBlockInfo newInfo = {currentInfo->dataBlockShmid,currentInfo->currentUsage-1};
            controlBlock_->unlock(0);
            controlBlock_->writeToSegment(0,&newInfo);
            return data;
        }
        else
        {
            throw  std::out_of_range("size of vector can't be less than 0");
        }
    }
    T at(int index)
    {
        assert(index>=0);
        auto currentInfo = (DataBlockInfo*)(controlBlock_->readFromSegment(0));
        controlBlock_->unlock(0);
        if(index < currentInfo->currentUsage)
        {
            T data = *((T*)dataBlock_->readFromSegment(index));
            dataBlock_->unlock(index);
            return data;
        }
        else
        {
            throw  std::out_of_range("Index out of range");
        }
    }
    T operator [](int index)
    {
        T data = *((T*)dataBlock_->readFromSegment(index));
        dataBlock_->unlock(index);
        return data;
    }
    bool empty()
    {
        auto currentInfo = (DataBlockInfo*)(controlBlock_->readFromSegment(0));
        controlBlock_->unlock(0);
        return currentInfo->currentUsage == 0;
    }
    void printData()
    {
        auto info = (DataBlockInfo*)(controlBlock_->readFromSegment(0));
        int range = info->currentUsage;
        for(int i = 0; i < range;++i)
        {
            std::cout<<at(i)<<std::endl;
        }
    }

private:
    void copyToNewDataBlock(int shmidForDataBlock,DataBlockInfo *info){
        auto dataBlock_tmp = std::make_unique<BaseShmm>(shmidForDataBlock);
        int range = info->currentUsage;
        for(int i = 0; i < range;++i)
        {
            T* data = (T*)dataBlock_->readFromSegment(i);
            dataBlock_tmp->writeToSegment(i,data);
            dataBlock_->unlock(i);
        }
        BaseShmm::deleteSegment(dataBlock_->getShmid());
        dataBlock_ = std::move(dataBlock_tmp);

    }
    std::unique_ptr<BaseShmm> controlBlock_; //place in memory where shmid for datablock is stored
    std::unique_ptr<BaseShmm> dataBlock_;
};

#endif // SHAREDVECTOR_H
