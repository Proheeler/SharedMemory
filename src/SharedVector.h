#ifndef SHAREDVECTOR_H
#define SHAREDVECTOR_H
#include "BaseShmm.h"
#include <memory>
#include <iostream>
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
    ~SharedVector(){
        BaseShmm::deleteSegment(controlBlock_->getShmid());
        BaseShmm::deleteSegment(dataBlock_->getShmid());
        controlBlock_.~unique_ptr();
        dataBlock_.~unique_ptr();
    }
    void push_back(T value)
    {
        auto currentInfo = (DataBlockInfo*)(controlBlock_->readFromSegment(0));
        if(currentInfo->currentUsage < (dataBlock_->getMaxSize()-2))
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
    void printData()
    {
        auto info = (DataBlockInfo*)(controlBlock_->readFromSegment(0));
        int range = info->currentUsage;
        for(int i = 0; i < range;++i)
        {
            T data = *((T*)dataBlock_->readFromSegment(i));
            std::cout<<data<<std::endl;
            dataBlock_->unlock(i);
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
