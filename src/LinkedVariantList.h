#ifndef LINKEDVARIANTLIST_H
#define LINKEDVARIANTLIST_H
#include <vector>
#include "VariantNode.h"
class LinkedVariantList
{
public:
    LinkedVariantList() = default;
    explicit LinkedVariantList(int headShmid):headShmid_(headShmid){}
private:
    int headShmid_{-1};
    std::vector<VariantNode> list_{};

};


#endif // LINKEDVARIANTLIST_H
