#pragma once


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <atomic>
#include <random>
#include <cstring>
#include <fcntl.h>
#include <stdexcept>
#include <fstream>
#include <set>
#include "ArrayStruct.h"
#include "SharedMemoryInfo.h"
/*!
    \brief Class for work with shared memory

    This class represents API for working with shared memory
*/

class BaseShmm
{
    /*!
    Iterator for shred memory class
    */
    class iterator
    {
        friend class BaseShmm;
        BaseShmm* ptr;
        int index;
    public:
        /*!
        Constructor for iterator
        \param[in] shmPtr Shared memory object of this library
        \param[in] index_ index of chunk in segment
        */
        iterator(BaseShmm* shmPtr,int64_t index_):ptr(shmPtr),index(index_){}
        void* operator*() const {
            void *retPtr = ptr->readFromSegment(index);
            ptr->unlock(index);
            return retPtr;
        }
        iterator& operator++() {
            ++index;
            return *this;
        }
        bool operator!=(const iterator &other) const
        {
            return index != other.index;
        }

    };
public:
    /*!
    Contructor that attachs shared memory segment to your program
    \param[in] shmid Shared memory identificator
    */
    explicit BaseShmm(int shmid_):shmid(shmid_)
    {
        if ((mem_ptr = static_cast<void*>(shmat(shmid, nullptr, 0))) == (void *) -1) {
            throw std::invalid_argument("Wrong Shared Memory Id passed,please check");
        }
        memorySize=getMemorySize();
        dataShift=getBlockSize()+sizeof(std::atomic<int>*);
        blockSize=getBlockSize();
        maxSize = (memorySize-2*sizeof(size_t*)-1)/dataShift;
    }
    /*!
    Desructor that deattachs shared memory segment from your program
    */
    ~BaseShmm()
    {
        shmdt(mem_ptr);
    }
    /*!
    Function for creation shared memory segment without attaching to program
    \param[in] memory_size Size in bytes for allocation
    \param[in] blockSize_ Size in bytes for data chunk
    */
    static int createSegment(size_t memory_size,size_t blockSize_)
    {
        void *tmp = nullptr;
        std::random_device rd;     // only used once to initialise (seed) engine
        std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
        std::uniform_int_distribution<int> uni(5000,10000); // guaranteed unbiased
        size_t maxSize = (memory_size-1-2*sizeof(size_t*))/(sizeof(std::atomic<int>*)+blockSize_);
        /* Create the segment */
        int shmidd=-1;
        while(shmidd < 0)
        {
            int key_ = uni(rng);
            shmidd = shmget(key_, memory_size, IPC_CREAT |  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            if(shmidd <0 && errno != EEXIST)
            {
                throw std::invalid_argument("Error creating shared memory segment");
            }

        }
        /* Attach the segment to our data space */
        if ((tmp = (void *) shmat(shmidd, nullptr, 0)) == (void *) -1) {
            throw std::invalid_argument("Wrong Shared Memory Id passed,please check");
        }
        char name = 's';
        memcpy(tmp,&name,1);

        for(size_t i=0;i<maxSize;++i){
            auto& atomic = (std::atomic<int>&)(*((std::atomic<int>*)(static_cast<char*>(tmp)+2*sizeof(size_t*)+1+i*(sizeof(std::atomic<int>*)+blockSize_))));
            atomic.store(0,std::memory_order_release);
        }
        auto& refForMemSize = static_cast<size_t&>(*(static_cast<size_t*>(static_cast<void*>(static_cast<char*>(tmp)+1))));
        refForMemSize = memory_size;
        auto& refForChunkSize = static_cast<size_t&>(*(static_cast<size_t*>(static_cast<void*>(static_cast<char*>(tmp)+1+sizeof(size_t*)))));
        refForChunkSize = blockSize_;
        shmdt(tmp);
        tmp=nullptr;
        return shmidd;
    }

    /*!
    Function that destruct shared memory segment
    \param[in] shmid_ Shared memory identificator
    */
    static int deleteSegment(int shmid_)
    {
        return shmctl(shmid_,IPC_RMID,nullptr);
    }

    /*!
    The function of reading one block of data, blocks a block of memory at a given index, you must then use the Unlock function
    \param[in] blockIndex Block index inside shared memory
    */
    auto readFromSegment(size_t blockIndex) -> void*
    {
        size_t index = blockIndex%(maxSize);
        auto& atomic = (std::atomic<int>&)(*((std::atomic<int>*)(static_cast<char*>(mem_ptr)+1+2*sizeof(size_t*)+index*dataShift)));
        void *ptr=nullptr;
        int free = 0;
        int read = 2;
        bool exchangedFree = atomic.compare_exchange_strong(free,read,std::memory_order_acquire,std::memory_order_release);
        bool exchangedRead = atomic.compare_exchange_strong(read,read,std::memory_order_acquire,std::memory_order_release);
        if(exchangedFree || exchangedRead)
        {
            ptr=static_cast<char*>(mem_ptr)+1+2*sizeof(size_t*)+index*dataShift+sizeof(std::atomic<int>*);
        }
        else
        {
            ptr = nullptr;
        }
        return ptr;
    }

    /*!
    Write function
    \param[in] blockIndex Block index inside shared memory
    \param[in] data Pointer to your data (Copy everything inside segment from this address till address+sizeof(data))
    */
    bool writeToSegment(size_t blockIndex,void *data)
    {
        size_t memory_size = getMemorySize();
        size_t blockSize_ = getBlockSize();
        size_t maxSize_=(memory_size-2*sizeof(int64_t*)-1)/(sizeof(std::atomic<int>*)+blockSize_);
        size_t dataShift_ =blockSize_+sizeof(std::atomic<int>*);
        size_t index = blockIndex%(maxSize_);
        volatile std::atomic<int>& atomic = (std::atomic<int>&)(*((std::atomic<int>*)(static_cast<char*>(mem_ptr)+1+2*sizeof(size_t*)+index*dataShift_)));
        int free = 0;
        int write = 1;
        bool exchanged = atomic.compare_exchange_strong(free,write);
        if(exchanged)
        {
            memcpy(static_cast<char*>(mem_ptr)+2*sizeof(size_t*)+1+index*dataShift_+sizeof(std::atomic<int>*),data,blockSize_);
            atomic.exchange(0,std::memory_order_acquire);
            return true;
        }
        return false;
    }

    /*!
    Function that unlock the data after readFromSegment function block
    \param[in] blockIndex Block index inside shared memory
    */
    void unlock(size_t blockIndex)
    {
        size_t memory_size = getMemorySize();
        size_t blockSize_ = getBlockSize();
        size_t maxSize_=(memory_size-2*sizeof(int64_t*)-1)/(sizeof(std::atomic<int>*)+blockSize_);
        size_t dataShift_ =blockSize_+sizeof(std::atomic<int>*);
        size_t index = blockIndex%(maxSize_);
        auto& atomic = (std::atomic<int>&)(*((std::atomic<int>*)(static_cast<char*>(mem_ptr)+1+2*sizeof(size_t*)+index*dataShift_)));
        atomic.exchange(0,std::memory_order_acquire);
    }

    /*!
    Returns size of data
    */
    size_t getBlockSize() const
    {
        auto& refForChunkSize = static_cast<size_t&>(*(static_cast<size_t*>(static_cast<void*>(static_cast<char*>(mem_ptr)+1+sizeof(size_t*)))));
        return refForChunkSize;
    }

    /*!
    Returns size of shared memory segment
    */
    size_t getMemorySize() const
    {
        auto& refForMemSize = static_cast<size_t&>(*(static_cast<size_t*>(static_cast<void*>(static_cast<char*>(mem_ptr)+1))));
        return refForMemSize;
    }

    /*!
    Returns number of data chunks inside memory
    */
    size_t getMaxSize() const
    {
        size_t memory_size = getMemorySize();
        size_t blockSize_ = getBlockSize();
        size_t maxSize_=(memory_size-2*sizeof(int64_t*)-1)/(sizeof(std::atomic<int>*)+blockSize_);
        return maxSize_;
    }

    /*!
    Checks if segment with id exists
    \param[in] shmid Идентификатор сегмента памяти
    */
    static bool checkShmid(int shmid)
    {
        void *tmp=nullptr;
        return ((tmp = static_cast<void*>(shmat(shmid, nullptr, 0))) == (void *) -1);
    }

    /*!
    Unblock all chunks
    */
    void resetAtomics()
    {
        for(size_t i=0;i<maxSize;++i){
            auto& atomic = (std::atomic<int>&)(*((std::atomic<int>*)(static_cast<char*>(mem_ptr)+1+2*sizeof(size_t*)+i*(sizeof(std::atomic<int>*)+blockSize))));
            atomic.store(0,std::memory_order_release);
        }
    }

    /*!
    Returns shared memory segment identificator
    */
    int getShmid() const
    {
        return shmid;
    }

    /*!
    Reinit Segment with new chunksize
    */
    void reinitMemory(const int & chunkSize)
    {
        void *tmp = mem_ptr;
        blockSize = chunkSize;
        maxSize = (getMemorySize()-2*sizeof(size_t*))/(sizeof(std::atomic<int>*)+blockSize);
        for(size_t i=0;i<maxSize;++i){
            auto& atomic = (std::atomic<int>&)(*((std::atomic<int>*)(static_cast<char*>(tmp)+1+2*sizeof(size_t*)+i*(sizeof(std::atomic<int>*)+blockSize))));
            atomic.store(0,std::memory_order_release);
        }
        size_t& refForChunkSize = static_cast<size_t&>(*(static_cast<size_t*>((void*)((char*)tmp+1+sizeof(size_t*)))));
        refForChunkSize = blockSize;
        dataShift=blockSize+sizeof(std::atomic<int>*);
    }
    /*!
    Find segments created by this library
    */
    static Array findSegments()
    {
        Array ret;
        using shmid = int;
        std::ifstream file;
        file.open("/proc/sysvipc/shm");
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::vector<shmid> v_info;
        if(file.fail())
        {
            return {};
        }
        while(!file.eof()){
            SharedMemoryInfo info;
            file>>info;
            if(info.perms == 666)
                v_info.push_back(info.shmid);
        }
        file.close();
        std::set<int> segments;
        for(size_t i = 0;i< v_info.size();++i)
        {
            void* tmp=nullptr;
            if ((tmp = (void *) shmat(v_info.at(i), NULL, 0)) == (void *) -1) {
                continue;
            }
            if(*static_cast<char*>(tmp) == 's'){
                segments.insert(v_info.at(i));
            }
            shmdt(tmp);
        }
        ret.size=segments.size();
        ret.pointer = new int[ret.size];

        for(auto i :segments)
        {
            *ret.pointer=i;
            ++ret.pointer;
        }
        ret.pointer-=ret.size;
        return ret;
    }

    iterator begin() const{
        iterator ret(const_cast<BaseShmm*>(this),0);
        return ret;
    }
    iterator end() const
    {
        iterator ret(const_cast<BaseShmm*>(this),getMaxSize());
        return ret;
    }
    void *get() const
    {
        return mem_ptr;
    }

private:
    size_t memorySize;
    size_t blockSize;
    size_t dataShift;
    size_t maxSize;
    void* mem_ptr;
    int shmid;
};


