#include "SharedMemoryInfo.h"

std::istream& operator >>(std::istream &stream,SharedMemoryInfo &info){
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
