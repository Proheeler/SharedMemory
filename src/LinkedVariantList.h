#ifndef LINKEDVARIANTLIST_H
#define LINKEDVARIANTLIST_H
#include <vector>
#include <iostream>
#include <cassert>
#include "VariantNode.h"
class SharedLinkedVariantList
{
public:
    SharedLinkedVariantList() = default;
    explicit SharedLinkedVariantList(int headShmid):headShmid_(headShmid){
        list_.push_back(VariantNode::createFromExisting(headShmid_));
        initStructure();
    }
    void pushbackNode(VariantNode node)
    {
        auto nodeInfo = node.info();
        nodeInfo.prevShmid_ = list_.back().info().currentShmid_;
        nodeInfo.nextShmid_ = -1;
        node.setInfo(nodeInfo);
        list_.push_back(node);
    }
    void pushbackNode(int shmid)
    {
        auto node = VariantNode::createFromExisting(shmid);
        auto nodeInfo = node.info();
        nodeInfo.prevShmid_ = list_.back().info().currentShmid_;
        nodeInfo.nextShmid_ = -1;
        node.setInfo(nodeInfo);
        list_.push_back(node);
    }
    void insertNode(size_t index,VariantNode node)
    {
        if(index!=0 && index-1<list_.size())
        {
            auto prevInfoprevNode = list_.at(index-1).info();
            prevInfoprevNode.nextShmid_ = node.info().currentShmid_;
            list_.at(index-1).setInfo(prevInfoprevNode);

            auto prevInfonextNode = list_.at(index).info();
            prevInfonextNode.prevShmid_ = node.info().currentShmid_;
            list_.at(index).setInfo(prevInfonextNode);

            auto nodeInfo = node.info();
            nodeInfo.prevShmid_ = prevInfoprevNode.currentShmid_;
            nodeInfo.nextShmid_ = prevInfonextNode.currentShmid_;
            node.setInfo(nodeInfo);

        }
        else if(index == 0)
        {
            auto prevInfonextNode = list_.at(index).info();
            prevInfonextNode.prevShmid_ = node.info().currentShmid_;
            list_.at(index).setInfo(prevInfonextNode);

            auto nodeInfo = node.info();
            nodeInfo.prevShmid_ = -1;
            nodeInfo.nextShmid_ = prevInfonextNode.currentShmid_;
            node.setInfo(nodeInfo);
        }
        else{
            index = list_.size();
            auto nodeInfo = node.info();
            nodeInfo.prevShmid_ = list_.back().info().currentShmid_;
            nodeInfo.nextShmid_ = -1;
            node.setInfo(nodeInfo);
        }
        list_.insert(list_.begin()+index,node);

    }
private:
    void initStructure(){
        NodeInfo info = list_.back().info();
        while(list_.back().info().nextShmid_ !=-1)
        {
            if(info.currentShmid_!=info.nextShmid_){
                list_.push_back(VariantNode::createFromExisting(list_.back().info().nextShmid_));
                std::cout<<"Insert node"<<std::endl;
                info = list_.back().info();
            }
            else{
                std::cout<<"Cycle detected"<<std::endl;
                break;
            }
        }

    }
    int headShmid_{-1};
    std::vector<VariantNode> list_{};

};


#endif // LINKEDVARIANTLIST_H
