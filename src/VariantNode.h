#ifndef VARIANTNODE_H
#define VARIANTNODE_H
#include "BaseShmm.h"
#include <memory>

struct NodeInfo
{
    int prevShmid_{-1};
    int nextShmid_{-1};
    int currentShmid_{-1};
    int dataSize_{-1};
};


struct VariantNode
{
    static VariantNode createFromExisting(int shmid){
        std::shared_ptr<BaseShmm> shmm = std::make_shared<BaseShmm>(shmid);
        void* data_ptr = shmm->readFromSegment(0);
        shmm->unlock(0);
        NodeInfo info = *(NodeInfo*)(data_ptr);
        VariantNode node;
        node.setShmm(shmm);
        node.setInfo(info);

        return node;
    }
    VariantNode() = default;
    explicit VariantNode(int dataSize){
        info_.currentShmid_ = BaseShmm::createSegment((sizeof(NodeInfo)+dataSize+sizeof(std::atomic<int>*))+1+2*sizeof(size_t),dataSize);
        info_.dataSize_ = dataSize;
        shmm_ = std::make_shared<BaseShmm>(info_.currentShmid_);
        writeNodeInfo();
    }
    ~VariantNode(){
        BaseShmm::deleteSegment(info_.currentShmid_);
    }
    NodeInfo info() const{
        return info_;
    }
    void setInfo(const NodeInfo &info){
        info_ = info;
        writeNodeInfo();
    }

    bool writeData(void *data)
    {
        void* mem_ptr = shmm_->get();
        volatile std::atomic<int>& atomic = (std::atomic<int>&)(*((std::atomic<int>*)(static_cast<char*>(mem_ptr)+1+2*sizeof(size_t*))));
        int free = 0;
        int write = 1;
        bool exchanged = atomic.compare_exchange_strong(free,write);
        if(exchanged)
        {
            memcpy(static_cast<char*>(mem_ptr)+2*sizeof(size_t*)+1+sizeof(NodeInfo)+sizeof(std::atomic<int>*),data,info_.dataSize_);
            atomic.exchange(0,std::memory_order_acquire);
            return true;
        }
        return false;
    }

    void setShmm(const std::shared_ptr<BaseShmm> &shmm){
        shmm_ = shmm;
    }

private:
    bool writeNodeInfo()
    {
        void* mem_ptr = shmm_->get();
        volatile std::atomic<int>& atomic = (std::atomic<int>&)(*((std::atomic<int>*)(static_cast<char*>(mem_ptr)+1+2*sizeof(size_t*))));
        int free = 0;
        int write = 1;
        bool exchanged = atomic.compare_exchange_strong(free,write);
        if(exchanged)
        {
            memcpy(static_cast<char*>(mem_ptr)+2*sizeof(size_t*)+1+sizeof(std::atomic<int>*),&info_,sizeof(NodeInfo));
            atomic.exchange(0,std::memory_order_acquire);
            return true;
        }
        return false;
    }
    NodeInfo info_;
    std::shared_ptr<BaseShmm> shmm_;
};


#endif // VARIANTNODE_H


