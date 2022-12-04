//
// Created by berg on 30/03/2020.
//

#ifndef BEVA_NODE_H
#define BEVA_NODE_H


#include <iostream>

#include <unordered_map>


using namespace std;
#define CHAR_SIZE 128

using namespace std;

class Node {
public:
    //    ShortVector<unsigned> children;
    unsigned children;
    unsigned beginRange;
    unsigned endRange;
    unsigned char numChildren;
    bool isEndOfWord;
    char value;
    double maxStaticScore;
    unsigned recordIdFromMaxScore;

    Node(char value) {
        this->numChildren = 0;
        this->maxStaticScore = 0.0;
        this->setValue(value);
        this->setIsEndOfWord(false);
    }

    Node() {
        this->numChildren = 0;
        this->maxStaticScore = 0.0;
        this->setIsEndOfWord(false);
    }

    inline void setValue(char value) { this->value = value; }
    inline char getValue() { return this->value; }
    inline void setIsEndOfWord(bool value) { this->isEndOfWord = value; }
    inline bool getIsEndOfWord() { return this->isEndOfWord; }
    inline unsigned getBeginRange() { return this->beginRange; }
    inline unsigned getEndRange() { return this->endRange;}
    inline void setBeginRange(unsigned value) { this->beginRange = value; }
    inline void setEndRange(unsigned value) { this->endRange = value; }
    inline void setMaxStaticScore(double value) { this->maxStaticScore = value; }
    inline double getMaxStaticScore() { return this->maxStaticScore; }
    inline void setRecordIdFromMaxScore(unsigned recordId) { this->recordIdFromMaxScore = recordId; }
    inline unsigned getRecordIdFromMaxScore() { return this->recordIdFromMaxScore; }
};

#endif //BEVA_NODE_H
