//
// Created by berg on 14/02/19.
//

#ifndef BEVA_BEVA_H
#define BEVA_BEVA_H


#include <unordered_map>
#include "Trie.h"
#include "ActiveNode.h"
#include "utils.h"
#include "TopKHeap.h"

#define CHAR_SIZE 128

using namespace std;

class Beva {
public:
    Trie* trie;
    Experiment *experiment;
    int editDistanceThreshold;
    int twiceEditDistanceThreshold;
    unsigned bitmapZero;
    unsigned long maskSum;
    unsigned long editVectorStartValue;
    unsigned long scalarMaskEdit;
    unsigned scalarDesloc;
    unsigned long editDistanceMasc;
    unordered_map<unsigned long, int> unaryToDecimalMap;
    unsigned char unaryToDecimalVet[32] = { 0 }; // until 32 for tau = 4
    long long *preCalculatedExponentiation;

    Beva(Trie*, Experiment*, int, long long *preCalculatedExponentiation);
    ~Beva();

    void processNoErrors(char ch,
                         int prefixQueryLength,
                         vector<ActiveNode>& oldNoErrorActiveNodes,
                         vector<ActiveNode>& currentNoErrorActiveNodes);

    void updateBitmap(char ch, unsigned bitmaps[CHAR_SIZE]);
    void processWithPruningV2(char ch,
                              int prefixQueryLength,
                              vector<ActiveNode>& oldActiveNodes,
                              vector<ActiveNode>& currentActiveNodes,
                              unsigned bitmaps[CHAR_SIZE],
                              TopKHeap& topKHeap);

    void findActiveNodesWithPruningV2(unsigned queryLength,
                                      ActiveNode &oldActiveNode,
                                      vector<ActiveNode> &activeNodes,
                                      unsigned bitmaps[CHAR_SIZE],
                                      TopKHeap& topKHeap);

    void process(char, int, vector<ActiveNode>& oldActiveNodes, vector<ActiveNode>& currentActiveNodes,
                 unsigned bitmaps[CHAR_SIZE]);
    void findActiveNodes(unsigned, ActiveNode&, vector<ActiveNode>&, unsigned bitmaps[CHAR_SIZE]);

    inline unsigned buildBitmap(unsigned queryLength, unsigned lastPosition, char c, unsigned bitmaps[CHAR_SIZE]) {
        int k = (int) queryLength - (int) lastPosition;
        return bitmaps[c] << (this->editDistanceThreshold - k);
    }

    inline unsigned long getNewEditVector(unsigned queryLength,
                                          unsigned long editVector,
                                          unsigned lastPosition,
                                          char c, unsigned bitmaps[CHAR_SIZE]) {
        unsigned bitmap = buildBitmap(queryLength, lastPosition, c,bitmaps);
        unsigned long newEditVector = (editVector >> 1) | (editVector << this->editDistanceThreshold);
        newEditVector &= this->maskSum;

        if (bitmap) {
            // In cases where there is a match between d[] and q[]
            // printf("editVector Original: %0lX\n", editVector);
            unsigned long mask = this->scalarMaskEdit;
            //	int x = 0;
            while (!(bitmap & 0x80000000)) {
                mask >>= this->scalarDesloc;
                bitmap <<= 1;
            }
            newEditVector = newEditVector | (editVector & mask);
            mask >>= this->scalarDesloc;
            bitmap = bitmap << 1;

            for (; bitmap; mask >>= this->scalarDesloc, bitmap <<= 1) {
                if (bitmap & 0x80000000) {
                    // maskEdit isolates only the element that we need to inspect
                    newEditVector = newEditVector | (editVector & mask);
                }
            }

            {
                unsigned long tmp;
                do {
                    tmp = newEditVector;
                    newEditVector |= (newEditVector >> (this->editDistanceThreshold + 2) & this->maskSum);
                } while (tmp != newEditVector);
            }
        }
        return newEditVector;
    }

    inline unsigned isActive(int pos, unsigned long vet) {
        if (pos > this->editDistanceThreshold) return 0;
        pos += this->editDistanceThreshold;
        if (vet & (0x8000000000000000 >> (((pos + 1) * (this->editDistanceThreshold + 1)) - 1))) {
            return 1;
        }
        return 0;
    }

    inline int retrieveEditDistance(int pos, unsigned long vet) {
        if (pos > (2 * this->editDistanceThreshold)) return this->editDistanceThreshold + 1;

        int numberOfDeslocMasc = (pos * (this->editDistanceThreshold + 1));
        unsigned long masc = this->editDistanceMasc >> numberOfDeslocMasc;
        unsigned long result = (vet & masc) >> (64 - (numberOfDeslocMasc + (this->editDistanceThreshold + 1)));

        int editDistance = this->unaryToDecimalVet[result];

        return editDistance;
    }
   
    void showEditVector(unsigned long vec) {
        unsigned cont = 0;
        int y;
        unsigned long masc;
        printf("( ");

        for(int x = 0 ; x < 2 * this->editDistanceThreshold + 1; x++) {
          masc=  0x8000000000000000 >> (x * (this->editDistanceThreshold + 1));
          for (y = 0; ((masc & vec) == 0) && y < this->editDistanceThreshold + 1; y++) { masc>>=1; }

          printf("%d ",y);
        }
        printf(")\n");
      }


 void showEditVector2(unsigned long vec) {
    unsigned cont = 0;
    int y;
    unsigned long masc;
    //    printf("( ");
    for(int x = 1 ; x < 2 * this->editDistanceThreshold; x++) {
      masc = 0x8000000000000000 >> (x * (this->editDistanceThreshold + 1));
      for(y = 0; ((masc & vec) == 0) && y < this->editDistanceThreshold + 1; y++) { masc>>=1; }
      
      printf("%d\n",y);
    }
    //    printf(")\n");
  }

  
};

#endif //BEVA_BEVA_H
