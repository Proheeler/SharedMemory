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
    ~SharedLinkedVariantList()
    {
        for(VariantNode const & i:list_)
        {
            BaseShmm::deleteSegment(i.info().currentShmid_);
        }
    }
    void pushback(VariantNode && node)
    {
        auto nodeInfo = node.info();
        nodeInfo.prevShmid_ = list_.back().info().currentShmid_;
        nodeInfo.nextShmid_ = -1;
        node.setInfo(nodeInfo);

        auto prevInfonextNode = list_.back().info();
        prevInfonextNode.nextShmid_ = node.info().currentShmid_;
        list_.back().setInfo(prevInfonextNode);

        list_.push_back(std::move(node));
    }
    void pushback(int shmid)
    {
        auto node = VariantNode::createFromExisting(shmid);
        auto nodeInfo = node.info();
        nodeInfo.prevShmid_ = list_.back().info().currentShmid_;
        nodeInfo.nextShmid_ = -1;
        node.setInfo(nodeInfo);

        auto prevInfonextNode = list_.back().info();
        prevInfonextNode.nextShmid_ = node.info().currentShmid_;
        list_.back().setInfo(prevInfonextNode);

        list_.push_back(std::move(node));
    }
    void insert(size_t index,VariantNode && node)
    {
        assert(index<list_.size());
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
            headShmid_ = node.info().currentShmid_;
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

            auto prevInfoprevNode = list_.at(index-1).info();
            prevInfoprevNode.nextShmid_ = node.info().currentShmid_;
            list_.at(index-1).setInfo(prevInfoprevNode);
            node.setInfo(nodeInfo);
        }
        list_.insert(list_.begin()+index,std::move(node));

    }
    void remove(size_t index){
        assert(index<list_.size());
        auto currentNodeInfo = list_.at(index).info();
        if(index != 0 && index+1<list_.size())
        {
            auto prevInfoprevNode = list_.at(index-1).info();
            auto prevInfonextNode = list_.at(index+1).info();
            prevInfoprevNode.nextShmid_ = prevInfonextNode.currentShmid_;
            list_.at(index-1).setInfo(prevInfoprevNode);
            prevInfonextNode.prevShmid_ = prevInfoprevNode.currentShmid_;
            list_.at(index+1).setInfo(prevInfonextNode);
        }
        else if(index == 0)
        {
            if(list_.size()>1){
                headShmid_ = list_.at(index+1).info().currentShmid_;
                auto prevInfonextNode = list_.at(index+1).info();
                prevInfonextNode.prevShmid_ = -1;
                list_.at(index+1).setInfo(prevInfonextNode);
            }
            else{
                headShmid_=-1;
            }
        }
        else{
            auto prevInfoprevNode = list_.at(index-1).info();
            prevInfoprevNode.nextShmid_ = -1;
            list_.at(index-1).setInfo(prevInfoprevNode);
        }
        list_.erase(list_.begin()+index);
        BaseShmm::deleteSegment(currentNodeInfo.currentShmid_);
    }
    VariantNode operator [](int index)
    {
        return list_.at(index);
    }
    VariantNode at(size_t index)
    {
        if(index<list_.size())
        {
            return list_.at(index);
        }
        throw std::invalid_argument("Index out of range");
    }
    size_t size()
    {
        return list_.size();
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
