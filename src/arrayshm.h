#ifndef ARRAYSHM_H
#define ARRAYSHM_H
#include "shmm_v1.h"
//#include "iteratorshm.h"
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
        std::atomic<int>& atomic = (std::atomic<int>&)(*((std::atomic<int>*)((char*)getMem_ptr()+2*sizeof(int64_t)+index*dataShift)));
        void *ptr=nullptr;
        int free = 0;
        int write = 1;
        bool exchanged = atomic.compare_exchange_strong(free,write);
        if(exchanged)
        {
            ptr=(char *)getMem_ptr()+2*sizeof(int64_t)+index*dataShift+sizeof(std::atomic<int>*);
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
    T* at(int index)
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
