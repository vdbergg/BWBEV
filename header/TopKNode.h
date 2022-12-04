#ifndef BEVA_TOPKNODE_H
#define BEVA_TOPKNODE_H

class TopKNode {
public:
    unsigned recordId;
    double maxScore;

    TopKNode(unsigned recordId, double maxScore)
    {
        this->setRecordId(recordId);
        this->setMaxScore(maxScore);
    }

    inline void setRecordId(unsigned recordId) { this->recordId = recordId; }
    inline unsigned getRecordId() { return this->recordId; }
    inline void setMaxScore(double maxScore) { this->maxScore = maxScore; }
    inline double getMaxScore() { return this->maxScore; }
};

#endif //BEVA_TOPKNODE_H