#ifndef SHAREDVECTOR_H
#define SHAREDVECTOR_H
#include "BaseShmm.h"

template< typename T>
class SharedVector:public BaseShmm
{
    explicit SharedVector(size_t numberOfElements):BaseShmm(
                                                   BaseShmm::createSegment((numberOfElements*(sizeof(T)+sizeof(std::atomic<int>*)))+1+2*sizeof(size_t),sizeof(T)))
    {
	resetAtomics();
    }
    ~SharedVector()
    {
	shmdt(get());
	BaseShmm::deleteSegment(getShmid());
    }
    bool write(int index,void* data,bool forceWrite = false)
    {
       auto isWritten = writeToSegment(index,data);
       if(!isWritten && forceWrite){
	   resetAtomics();
	   isWritten =writeToSegment(index,data);
       }
       return isWritten;
    }
    template<typename ... U>
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
    T* operator [](int index)
    {
	return static_cast<T*>(readFromSegment(index));
    }
    T* at(size_t index)
    {
	if(index<getMaxSize() && index>=0)
	{
	    return (static_cast<T*>(readFromSegment(index)));
	}
	throw std::invalid_argument("Index out of range");
    }
    size_t size(){
	return getMaxSize();
    }
};

#endif // SHAREDVECTOR_H
