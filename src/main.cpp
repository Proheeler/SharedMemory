#include <iostream>
#include <string.h>

////#include <opencv2/opencv.hpp>
#include "shmm_v1.h"
#include "ext.h"

int main()
{

    // w h  chz 200 frames

    int shmid =Create(1920*1080*250,1920*1080);

//    my_array arr;
//   arr = FindSegments();
////   std::cout<<arr.size<<std::endl;
//   for(int i=0;i<arr.size;++i)
//   {
//       std::cout<<i<<" "<<*(arr.pointer+i)<<std::endl;
//   }
//   FreeResultOfFindSegments(arr.pointer);
////   delete []arr.pointer;
//  //  int shmid_vid =Create(720*486*1*200+1600,720*486);


//    BaseShmm* shmm = BaseShmm_Create(shmid);
//    std::cout<<shmid<<std::endl;
////    std::cout<<shmm->getBlockSize()<<"\n"<<shmm->getMemorySize()<<std::endl;
//    char a[]="1234567890";
//    Write(shmm,0,a);
//    std::string tmp((char*)(LockAndRead(shmm,0)));

//    std::cout<<tmp<<std::endl;
////    std::cout<<checkShmid(shmid)<<std::endl;
////    BaseShmm_Delete(shmm);
//    delete shmm;
//    BaseShmm::deleteSegment(shmid);
////

   // std::cout<<"shm id video: "<<shmid_vid<<std::endl;
    return 0;
}

