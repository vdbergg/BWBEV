//
// Created by berg on 09/02/19.
//

#ifndef BEVA_ACTIVENODE_H
#define BEVA_ACTIVENODE_H


class ActiveNode {
public:
    unsigned long editVector;
    unsigned node;
    unsigned level;
    int editDistance;

    ActiveNode(unsigned, unsigned long, unsigned, int);
    ActiveNode(unsigned, unsigned long, unsigned);
};

#endif //BEVA_ACTIVENODE_H
