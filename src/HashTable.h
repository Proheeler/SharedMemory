#ifndef HASHTABLE_H
#define HASHTABLE_H
#include "BaseShmm.h"
#include <memory>
/*
Should work smthg like this
               indexes->LinkedLists
               indexes->LinkedLists
controlBlock-> indexes->LinkedLists
               indexes->LinkedLists
               indexes->LinkedLists

Question is whta hashing function to use
*/
class HashTable
{
public:
    HashTable(){}
    HashTable(int shmidForControlBlock)
    {
        controlBlock_ = std::make_unique<BaseShmm>(shmidForControlBlock);
    }
private:
    std::unique_ptr<BaseShmm> controlBlock_; //place in memory where shmid for index block
    std::unique_ptr<BaseShmm> indexBlock_;


};

#endif // HASHTABLE_H
