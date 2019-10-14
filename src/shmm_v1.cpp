#include "shmm_v1.h"
#include <iostream>
#include <cstring>
#include <random>
#include <fstream>
#include <limits>
#include <vector>
#include <set>
struct my_shm_info
{
    int key,shmid ,perms,size,cpid,lpid,nattch,uid,gid,cuid,cgid,atime,dtime,ctime ,rss,swap;

};

std::istream& operator >>(std::istream &stream,my_shm_info &info)
{
      stream>>info.key>>info.shmid
            >>info.perms>>info.size
            >>info.cpid>>info.lpid
            >>info.nattch>>info.uid
            >>info.gid>>info.cuid
            >>info.cgid>>info.atime
            >>info.dtime>>info.ctime
            >>info.rss>>info.swap;
    return stream;
}




BaseShmm::BaseShmm(int shmid_):sizeOfInt64(sizeof(int64_t*)),shmid(shmid_)
{
    if ((mem_ptr = (void *)shmat(shmid, NULL, 0)) == (void *) -1) {
        throw std::invalid_argument("System dont give memory =(((");
    }
    memorySize=getMemorySize();
    dataShift=getBlockSize()+sizeof(std::atomic<int>*);;
    blockSize=getBlockSize();
    maxSize = (memorySize-2*sizeOfInt64-1)/dataShift;
    std::cout<<"new version"<<std::endl;
}

BaseShmm::~BaseShmm()
{
    shmdt(mem_ptr);
}

int BaseShmm::createSegment(int64_t memory_size, int64_t blockSize_)
{
    void *tmp = nullptr;
    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uni(5000,10000); // guaranteed unbiased

    int key_ = uni(rng);
    int maxSize = (memory_size-1-2*sizeof(int64_t*))/(sizeof(std::atomic<int>*)+blockSize_);
    /* Create the segment */
    int shmidd;
    if ((shmidd = shmget(key_, memory_size, IPC_CREAT | 0666)) < 0) {
        throw std::invalid_argument("System dont give memory =(((");
    }
    /* Attach the segment to our data space */
    if ((tmp = (void *) shmat(shmidd, NULL, 0)) == (void *) -1) {
        throw std::invalid_argument("Invalid shmid passed");
    }
    char name = 's';
    memcpy(tmp,&name,1);
    for(int i=0;i<maxSize;++i){
        std::atomic<int>& atomic = (std::atomic<int>&)(*((std::atomic<int>*)((char*)tmp+2*sizeof(int64_t*)+1+i*(sizeof(std::atomic<int>*)+blockSize_))));
        atomic.store(0,std::memory_order_release);
    }
    int64_t& refForMemSize = static_cast<int64_t&>(*(static_cast<int64_t*>((void*)((char*)tmp+1))));
    refForMemSize = memory_size;
    int64_t& refForChunkSize = static_cast<int64_t&>(*(static_cast<int64_t*>((void*)((char*)tmp+1+sizeof(int64_t*)))));
    refForChunkSize = blockSize_;
    tmp=nullptr;
    return shmidd;
}


int BaseShmm::deleteSegment(int shmid_)
{
    return shmctl(shmid_,IPC_RMID,NULL);
}


void* BaseShmm::readFromSegment(int64_t blockIndex)
{
    int64_t index = blockIndex%(maxSize);
    std::atomic<int>& atomic = (std::atomic<int>&)(*((std::atomic<int>*)((char*)mem_ptr+1+2*sizeOfInt64+index*dataShift)));
    void *ptr=nullptr;
    int free = 0;
    int read = 2;
    bool exchangedFree = atomic.compare_exchange_strong(free,read,std::memory_order_acquire,std::memory_order_release);
    bool exchangedRead = atomic.compare_exchange_strong(read,read,std::memory_order_acquire,std::memory_order_release);
    if(exchangedFree || exchangedRead)
    {
        ptr=(char *)mem_ptr+1+2*sizeOfInt64+index*dataShift+sizeof(std::atomic<int>*);
    }
    else
    {
        std::cout<<"rec locked 1"<<std::endl;
        ptr = nullptr;
    }
    return ptr;
}



bool BaseShmm::writeToSegment(int64_t blockIndex, void *data)
{
    int64_t index = blockIndex%(maxSize);
    volatile std::atomic<int>& atomic = (std::atomic<int>&)(*((std::atomic<int>*)((char*)mem_ptr+1+2*sizeOfInt64+index*dataShift)));
    int free = 0;
    int write = 1;
    bool exchanged = atomic.compare_exchange_strong(free,write);
    if(exchanged)
    {
        memcpy((char*)mem_ptr+2*sizeOfInt64+1+index*dataShift+sizeof(std::atomic<int>*),data,blockSize);
        atomic.exchange(0,std::memory_order_acquire);
        return true;
    }
    else
    {
        return false;
    }
}


void BaseShmm::unlock(int64_t frameNum)
{
    int64_t index = frameNum%(maxSize);
    std::atomic<int>& atomic = (std::atomic<int>&)(*((std::atomic<int>*)((char*)mem_ptr+1+2*sizeOfInt64+index*dataShift)));
    atomic.exchange(0,std::memory_order_acquire);
}


int64_t BaseShmm::getBlockSize() const
{
    int64_t& refForChunkSize = static_cast<int64_t&>(*(static_cast<int64_t*>((void*)((char*)mem_ptr+1+sizeof(int64_t*)))));
    return refForChunkSize;
}

int64_t BaseShmm::getMemorySize() const
{
    int64_t& refForMemSize = static_cast<int64_t&>(*(static_cast<int64_t*>((void*)((char*)mem_ptr+1))));
    return refForMemSize;
}

int BaseShmm::getMaxSize() const
{
    return maxSize;
}

bool BaseShmm::checkShmid(int shmid)
{
    void *tmp=nullptr;
    return ((tmp = (void *) shmat(shmid, NULL, 0)) != (void *) -1);
}

void BaseShmm::resetAtomics()
{
    for(int i=0;i<maxSize;++i){
        std::atomic<int>& atomic = (std::atomic<int>&)(*((std::atomic<int>*)((char*)mem_ptr+1+2*sizeof(int64_t*)+i*(sizeof(std::atomic<int>*)+blockSize))));
        atomic.store(0,std::memory_order_release);
    }
}

int BaseShmm::getShmid() const
{
    return shmid;
}

my_array BaseShmm::findSegments()
{
    my_array ret;
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
        my_shm_info info;
        file>>info;
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
