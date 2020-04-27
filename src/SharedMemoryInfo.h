#pragma once
#include <istream>


struct SharedMemoryInfo
{
    int key,shmid ,perms,size,cpid,lpid,nattch,uid,gid,cuid,cgid,atime,dtime,ctime ,rss,swap;
};

std::istream& operator >>(std::istream &stream,SharedMemoryInfo &info);

