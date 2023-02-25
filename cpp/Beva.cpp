//
// Created by berg on 14/02/19.
//

#include "../header/Beva.h"
#include "../header/ActiveNode.h"
#include "../header/utils.h"

using namespace std;


Beva::Beva(Trie* trie, Experiment* experiment, int editDistanceThreshold, long long *preCalculatedExponentiation) {
    this->editDistanceThreshold = editDistanceThreshold;
    this->twiceEditDistanceThreshold = 2 * editDistanceThreshold;
    this->preCalculatedExponentiation = preCalculatedExponentiation;
    // Bellow:
    //   fix length unary numeric system where a number is the amount of zeros it contains. Example: 011 is one; 001 is two.
    // Edit vector is represented in (2*tau + 1) positions with tau+1 bits for each position. For instance, tau =3 then we have (2*3+1=7) positions and each position has 4 bits, thus able to represent numbers from 0 to 4. The total  is 28 bits
    // Start value for 3 erros is 3 2 1 0 1 2 3, using the coding above.
    //maskSum is adopted to "add" an extra zero to each element of the edit vector. We make a shit >>1 and then and (&) with the mask.
    switch(this->editDistanceThreshold) {
        case 1:
            this->maskSum = 0x5400000000000000; // Adopted in the sum. 01 01 01 000...
            this->editVectorStartValue = 0x7400000000000000; // 01 11 01 00
            this->editDistanceMasc = 0xC000000000000000; // 11000
            unaryToDecimalMap[3] = 0;
            unaryToDecimalMap[1] = 1;
            unaryToDecimalMap[0] = 2;

            unaryToDecimalVet[3] = 0;
            unaryToDecimalVet[1] = 1;
            unaryToDecimalVet[0] = 2;
            break;
        case 2:
            this->maskSum = 0x6DB6C00000000000; // 011 011 011 011 011 00 00...
            this->editVectorStartValue = 0x2FB2000000000000; // 001 011 111 011 001 0
            this->editDistanceMasc = 0xE000000000000000; // 11100
            unaryToDecimalMap[7] = 0;
            unaryToDecimalMap[3] = 1;
            unaryToDecimalMap[1] = 2;
            unaryToDecimalMap[0] = 3;
            unaryToDecimalVet[7] = 0;
            unaryToDecimalVet[3] = 1;
            unaryToDecimalVet[1] = 2;
            unaryToDecimalVet[0] = 3;
            break;
        case 3:
            this->maskSum = 0x7777777000000000; // 0111 0111 0111 0111 0111 0111 0111
            this->editVectorStartValue = 0x137F731000000000; // 0001 0011 0111 1111 0111 0011 0001
            this->editDistanceMasc = 0xF000000000000000; // 11110
            unaryToDecimalMap[15] = 0;
            unaryToDecimalMap[7] = 1;
            unaryToDecimalMap[3] = 2;
            unaryToDecimalMap[1] = 3;
            unaryToDecimalMap[0] = 4;

            unaryToDecimalVet[15] = 0;
            unaryToDecimalVet[7] = 1;
            unaryToDecimalVet[3] = 2;
            unaryToDecimalVet[1] = 3;
            unaryToDecimalVet[0] = 4;
            break;
        case 4:
            this->maskSum = 0x7BDEF7BDEF780000; // 0111 1011 1101 1110 1111 0111 1011 1101 1110 1111 0111 1000
            this->editVectorStartValue = 0x08CEFFBCE3080000;   // 0000 1000 1100 1110 1111 1111 1011 1100 1110 0011 0000 1000 00000
            this->editDistanceMasc = 0x1F00000000000000; // 11111
            unaryToDecimalMap[31] = 0;
            unaryToDecimalMap[15] = 1;
            unaryToDecimalMap[7] = 2;
            unaryToDecimalMap[3] = 3;
            unaryToDecimalMap[1] = 4;
            unaryToDecimalMap[0] = 5;

            unaryToDecimalVet[31] = 0;
            unaryToDecimalVet[15] = 1;
            unaryToDecimalVet[7] = 2;
            unaryToDecimalVet[3] = 3;
            unaryToDecimalVet[1] = 4;
            unaryToDecimalVet[0] = 5;
            break;
        default:
            cout << this->editDistanceThreshold << endl;
            cout << "Maximum Number of Errors is 4 in this version\n";
            exit(1);
    }

    this->scalarMaskEdit = (0xFFFFFFFFFFFFFFFF >> (64 - (this->editDistanceThreshold + 1)));
    this->scalarMaskEdit <<= (64 - (this->editDistanceThreshold + 1));
    this->scalarDesloc = this->editDistanceThreshold + 1;
    this->trie = trie;
    this->bitmapZero = 0;
    this->experiment = experiment;
}

Beva::~Beva() {

}

void Beva::processNoErrors(char ch,
                           int prefixQueryLength,
                           vector<ActiveNode>& oldActiveNodes,
                           vector<ActiveNode>& currentActiveNodes) {
    if (prefixQueryLength == 1) {
        oldActiveNodes.emplace_back(this->trie->root, this->editVectorStartValue, 0, 0);
    }

    for (ActiveNode oldActiveNode : oldActiveNodes) {
        unsigned child = this->trie->getNode(oldActiveNode.node).children;
        unsigned endChildren = child + this->trie->getNode(oldActiveNode.node).numChildren;
        unsigned tempSize = oldActiveNode.level + 1;

        for (; child < endChildren; child++) {
            if (ch == this->trie->getNode(child).getValue()) {
                #ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
                this->experiment->incrementNumberOfActiveNodes(queryLength);
                #endif
                currentActiveNodes.emplace_back(child, this->editVectorStartValue, tempSize, 0);
                break;
            }
        }
    }
}

void Beva::processWithPruningV2(char ch,
                                int prefixQueryLength,
                                vector<ActiveNode>& oldActiveNodes,
                                vector<ActiveNode>& currentActiveNodes,
                                unsigned bitmaps[CHAR_SIZE],
                                TopKHeap& heap) {
    this->updateBitmap(ch, bitmaps);

    if (prefixQueryLength == 1) {
        double childScore = utils::dynamicScore(this->trie->getNode(this->trie->root).getMaxStaticScore(),
                                                this->preCalculatedExponentiation[this->editDistanceThreshold]);
        unsigned recordIdFromActiveNodeScore = this->trie->getNode(this->trie->root).getRecordIdFromMaxScore();

        if (heap.isFull() && (childScore < heap.topMaxScore() || heap.contains(recordIdFromActiveNodeScore))) return;

        currentActiveNodes.emplace_back(this->trie->root, this->editVectorStartValue, 0, prefixQueryLength);
        #ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
        this->experiment->incrementNumberOfActiveNodes(query.length());
        #endif
    } else if (prefixQueryLength > this->editDistanceThreshold) {
        for (ActiveNode oldActiveNode : oldActiveNodes) {
            this->findActiveNodesWithPruningV2(prefixQueryLength,
                                               oldActiveNode,
                                               currentActiveNodes,
                                               bitmaps,
                                               heap);
        }
    } else {
        swap(currentActiveNodes, oldActiveNodes);
    }
}

void Beva::findActiveNodesWithPruningV2(unsigned queryLength,
                                        ActiveNode &oldActiveNode,
                                        vector<ActiveNode> &activeNodes,
                                        unsigned bitmaps[CHAR_SIZE],
                                        TopKHeap& topKHeap) {
    unsigned child = this->trie->getNode(oldActiveNode.node).children;
    unsigned endChildren = child + this->trie->getNode(oldActiveNode.node).numChildren;
    unsigned tempSize = oldActiveNode.level + 1;

    double nodeScore = utils::dynamicScore(this->trie->getNode(oldActiveNode.node).getMaxStaticScore(),
                                           this->preCalculatedExponentiation[this->editDistanceThreshold]);
    unsigned recordIdFromNodeScore = this->trie->getNode(oldActiveNode.node).getRecordIdFromMaxScore();
    if (topKHeap.isFull() && (nodeScore < topKHeap.topMaxScore() || topKHeap.contains(recordIdFromNodeScore))) return;

    for (; child < endChildren; child++) {
        #ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
        this->experiment->incrementNumberOfIterationInChildren(queryLength);
        #endif

        unsigned long newEditVector = this->getNewEditVector(queryLength,
                                                             oldActiveNode.editVector,
                                                             tempSize,
                                                             this->trie->getNode(child).getValue(),
                                                             bitmaps);


        if (newEditVector == 0) continue; // Final state is 00000000000

        #ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
        this->experiment->incrementNumberOfActiveNodes(queryLength);
        #endif

        if (this->isActive((int) queryLength - (int) tempSize, newEditVector)) {
            activeNodes.emplace_back(child, newEditVector, tempSize);
//        int currentEditDistance = this->retrieveEditDistance(((int) queryLength - (int) tempSize) + this->editDistanceThreshold, newEditVector);
//        if (currentEditDistance <= this->editDistanceThreshold) {
//            activeNodes.emplace_back(child, newEditVector, tempSize, currentEditDistance);
        } else {
            ActiveNode tmp(child, newEditVector, tempSize);
            this->findActiveNodesWithPruningV2(queryLength, tmp, activeNodes, bitmaps, topKHeap);
        }
    }
}

void Beva::process(char ch,
                   int prefixQueryLength,
                   vector<ActiveNode>& oldActiveNodes,
		               vector<ActiveNode>& currentActiveNodes,
		               unsigned bitmaps[CHAR_SIZE]) {
    this->updateBitmap(ch, bitmaps);

    if (prefixQueryLength == 1) {
        currentActiveNodes.emplace_back(this->trie->root, this->editVectorStartValue, 0, prefixQueryLength);
        #ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
        this->experiment->incrementNumberOfActiveNodes(query.length());
        #endif
    } else if (prefixQueryLength > this->editDistanceThreshold) {
        for (ActiveNode oldActiveNode : oldActiveNodes) {
	        this->findActiveNodes(prefixQueryLength, oldActiveNode, currentActiveNodes, bitmaps);
        }
    } else {
        swap(currentActiveNodes, oldActiveNodes);
        for (ActiveNode& activeNode : currentActiveNodes) { // This code is necessary when the edit distance were obtained in fetching phase
            activeNode.editDistance++;
        }
    }
}

// bitmap in bshift starts at the last bit, so does not need to be filled
// with zeros  after the shift.

void Beva::updateBitmap(char ch, unsigned bitmaps[CHAR_SIZE]) { // query is equivalent to Q' with the last character c
  for (unsigned x = 0; x < CHAR_SIZE; x++) {
      bitmaps[x] <<= 1;
  }
  bitmaps[ch] = bitmaps[ch] | (0x80000000 >> this->twiceEditDistanceThreshold);
}

void Beva::findActiveNodes(unsigned queryLength,
                           ActiveNode &oldActiveNode,
			                     vector<ActiveNode> &activeNodes,
			                     unsigned bitmaps[CHAR_SIZE]) {
  unsigned child = this->trie->getNode(oldActiveNode.node).children;
  unsigned endChildren = child + this->trie->getNode(oldActiveNode.node).numChildren;
  
  unsigned tempSize = oldActiveNode.level + 1;

  for (; child < endChildren; child++) {

    #ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
        this->experiment->incrementNumberOfIterationInChildren(queryLength);
    #endif

    unsigned long newEditVector = this->getNewEditVector(queryLength,
							 oldActiveNode.editVector, 
							 tempSize, 
							 this->trie->getNode(child).getValue(),
							 bitmaps);
    

    if (newEditVector == 0) continue; // Final state is 00000000000
    

    #ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
        this->experiment->incrementNumberOfActiveNodes(queryLength);
    #endif
    
    if (this->isActive((int) queryLength - (int) tempSize, newEditVector)) {
      activeNodes.emplace_back(child, newEditVector, tempSize);
    } else {
      ActiveNode tmp(child, newEditVector, tempSize);
      this->findActiveNodes(queryLength, tmp, activeNodes, bitmaps);
    } 
  }
}
