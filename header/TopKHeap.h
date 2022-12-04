#include <vector>
#include "TopKNode.h"
#include <bits/stdc++.h>


#ifndef BEVA_TOPKHEAP_H
#define BEVA_TOPKHEAP_H

using namespace std;


class myComparator {
public:
    bool operator() (TopKNode& p1, TopKNode& p2) {
        return p1.getMaxScore() > p2.getMaxScore();
    }
};

class TopKHeap {
public:
    priority_queue<TopKNode&, vector<TopKNode>, myComparator> heap;
    unordered_set<unsigned> recordIdsIntoHeap;
    unsigned capacity;

    TopKHeap(unsigned capacity);
    ~TopKHeap();

    void insertNode(TopKNode& node);
    vector<char *> outputSuggestions();
    bool contains(unsigned recordId);
    bool contains(TopKNode& node);
    bool isFull();
    double topMaxScore();
    void print();
};

#endif //BEVA_TOPKHEAP_H
