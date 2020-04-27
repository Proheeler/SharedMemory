#ifndef ARRAYSHM_H
#define ARRAYSHM_H
#include "BaseShmm.h"
class ArrayShm:public BaseShmm
{
public:
    explicit ArrayShm(int shmid):BaseShmm(shmid){
        resetAtomics();
    }

    template<typename T,typename ... U>
    bool emplaceWrite(int index,const U ...args)
    {
        int dataShift=getBlockSize()+sizeof(std::atomic<int>*);
        std::atomic<int>& atomic = (std::atomic<int>&)(*((std::atomic<int>*)(static_cast<char*>(get())+1+2*sizeof(size_t)+index*dataShift)));
        void *ptr=nullptr;
        int free = 0;
        int write = 1;
        bool exchanged = atomic.compare_exchange_strong(free,write);
        if(exchanged)
        {
            ptr=static_cast<char*>(get())+1+2*sizeof(size_t)+index*dataShift+sizeof(std::atomic<int>*);
            new (ptr) T{args ...};
            unlock(index);
            return true;
        }
        return false;

    }
    template<typename T>
    T* operator [](int index)
    {
        return static_cast<T*>(readFromSegment(index));
    }
    template<typename T>
    T* at(size_t index)
    {
        if(index<getMaxSize() && index>=0)
        {
            return static_cast<T*>(readFromSegment(index));
        }
        throw std::invalid_argument("Index out of range");
    }
    size_t size(){
        return getMaxSize();
    }
};

#endif // ARRAYSHM_H
