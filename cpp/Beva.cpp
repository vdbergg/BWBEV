//
// Created by berg on 14/02/19.
//

#include "../header/Beva.h"
#include "../header/ActiveNode.h"
#include "../header/utils.h"

using namespace std;


Beva::Beva(Trie *trie, Experiment* experiment, int editDistanceThreshold) {
  this->editDistanceThreshold = editDistanceThreshold;
  this->twiceEditDistanceThreshold= 2* editDistanceThreshold;
  // Bellow:
  //   fix length unary numeric system where a number is the amount of zeros it contains. Example: 011 is one; 001 is two.
  // Edit vector is represented in (2*tau + 1) positions with tau+1 bits for each position. For instance, tau =3 then we have (2*3+1=7) positions and each position has 4 bits, thus able to represent numbers from 0 to 4. The total  is 28 bits
  // Start value for 3 erros is 3 2 1 0 1 2 3, using the coding above.
  //maskSum is adopted to "add" an extra zero to each element of the edit vector. We make a shit >>1 and then and (&) with the mask.
  switch(this->editDistanceThreshold) {
    case 1:
        this->maskSum=0x5400000000000000; // Adopted in the sum. 01 01 01 000...
        this->editVectorStartValue =0x7400000000000000; // 01 11 01 00
        break;
    case 2:
        this->maskSum=0x6DB6C00000000000; // 011 011 011 011 011 00 00...
        this->editVectorStartValue =0x2FB2000000000000; // 001 011 111 011 001 0
        break;
    case 3:
        this->maskSum=0x7777777000000000; // 0111 0111 0111 0111 0111 0111 0111
        this->editVectorStartValue =0x137F731000000000; // 0001 0011 0111 1111 0111 0011 0001
        break;
    case 4:
        this->maskSum=0x7BDEF7BDEF780000; // 0111 1011 1101 1110 1111 0111 1011 1101 1110 1111 0111 1000
        this->editVectorStartValue =0x08CEFFBCE3080000;   // 0000 1000 1100 1110 1111 1111 1011 1100 1110 0011 0000 1000 00000
        break;
    default:
        cout << "Maximum Number of Errors is 4 in this version\n";
        exit(1);
  }

  this->maskEdit = new unsigned long[2 * this->editDistanceThreshold + 1];

  for (int x = 0; x <= 2 * this->editDistanceThreshold; x++) {
    unsigned long l1,l2;
    //example to v[0] (maskEdit[0])
    // result: 00000-11111-11111-11111-....
    // example to v[1] (maskEdit[1])
    // result: 11111-00000-11111-11111-11111-....
    // example to v[2] (maskEdit[2])
    // result: 11111-11111-00000-11111-11111-11111-....


    this->maskEdit[x] = (0xFFFFFFFFFFFFFFFF >>(64-(this->editDistanceThreshold+1)));
    this->maskEdit[x] = this->maskEdit[x] <<(64-((x+1)*(this->editDistanceThreshold+1)));
    //    printf("[%0lX]\n", this->maskEdit[x]);
  }
  this->bitmapSize = (1 << ((2 * this->editDistanceThreshold) + 1)) - 1; // 2^(2tau + 1) - 1
  this->trie = trie;
  this->bitmapZero = 0;
  this->bitmapOne = 1;
  this->experiment = experiment;
}

Beva::~Beva() {

}

void Beva::process(char ch, int prefixQueryLength, vector<ActiveNode>& oldActiveNodes,
		   vector<ActiveNode>& currentActiveNodes, unsigned bitmaps[CHAR_SIZE]) {
  this->updateBitmap(ch, bitmaps);

    if (prefixQueryLength == 1) {
      currentActiveNodes.emplace_back(this->trie->root, this->editVectorStartValue, 0);
#ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
      this->experiment->incrementNumberOfActiveNodes(query.length());
#endif
    } else if (prefixQueryLength > this->editDistanceThreshold) {
        for (ActiveNode oldActiveNode : oldActiveNodes) {
	        this->findActiveNodes(prefixQueryLength, oldActiveNode, currentActiveNodes, bitmaps);
        }
    } else {
      swap(currentActiveNodes, oldActiveNodes);
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


void Beva::findActiveNodes(unsigned queryLength, ActiveNode &oldActiveNode,
			   vector<ActiveNode> &activeNodes, unsigned bitmaps[CHAR_SIZE]) {
  
  unsigned child = this->trie->getNode(oldActiveNode.node).children;
  unsigned endChilds = child + this->trie->getNode(oldActiveNode.node).numChildren;
  
  unsigned tempSize = oldActiveNode.level + 1;

  for (; child < endChilds; child++) {

    #ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
        this->experiment->incrementNumberOfIterationInChildren(queryLength);
    #endif

    unsigned long newEditVector = this->getNewEditVector(queryLength,
							 oldActiveNode.editVector, 
							 tempSize, 
							 this->trie->getNode(child).getValue(),bitmaps);
    

    if (newEditVector == 0) continue; // Final state is 00000000000
    

#ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
    this->experiment->incrementNumberOfActiveNodes(queryLength);
#endif
    

    if (this->isActive((int) queryLength - (int) tempSize, newEditVector)) {

      activeNodes.emplace_back(child, newEditVector, tempSize);
    } else {

      ActiveNode tmp(child, newEditVector, tempSize);
      this->findActiveNodes(queryLength, tmp, activeNodes,bitmaps);
    } 
  }
}
