//
// Created by berg on 14/02/19.
//

#ifndef BEVA_BEVA_H
#define BEVA_BEVA_H


#include "EditVectorAutomata.h"
#include "Trie.h"
#include "ActiveNode.h"
#include "utils.h"

#define CHAR_SIZE 128

class Beva {
public:
    Trie* trie;
    Experiment *experiment;
    int editDistanceThreshold;
    int twiceEditDistanceThreshold;
    int bitmapSize;
    unsigned bitmapZero;
    unsigned bitmapOne;
    unsigned long maskSum;
    unsigned long editVectorStartValue;
    unsigned long *maskEdit;


    Beva(Trie*, Experiment*, int);
    ~Beva();
    void updateBitmap(char ch, unsigned bitmaps[CHAR_SIZE]);
    void process(char, int, vector<ActiveNode>& oldActiveNodes, vector<ActiveNode>& currentActiveNodes,
            unsigned bitmaps[CHAR_SIZE]);
    void findActiveNodes(unsigned, ActiveNode&, vector<ActiveNode>&, unsigned bitmaps[CHAR_SIZE]);
    
    inline unsigned buildBitmap(unsigned queryLength, unsigned lastPosition, char c, unsigned bitmaps[CHAR_SIZE]) {

      int k = (int) queryLength - (int) lastPosition;
      return bitmaps[c]<<(this->editDistanceThreshold - k);
    }
  
    inline unsigned long getNewEditVector(unsigned queryLength, 
					  unsigned long editVector, 
					  unsigned lastPosition, 
					  char c, unsigned bitmaps[CHAR_SIZE]) {
      unsigned bitmap = buildBitmap(queryLength, lastPosition, c,bitmaps);
      unsigned long newEditVector;
      
      /*    printf("Edit Vector de Entrada:");
	    showEditVector(editVector);
      */
      newEditVector = (editVector >> 1)|(editVector << this->editDistanceThreshold);
      newEditVector&=this->maskSum;
      if(bitmap) {
	// In cases where there is a match between d[] and q[]
	// printf("editVector Original: %0lX\n", editVector);
	// May it be paralelized ????
	unsigned mask = 0x80000000; // to isolate bitmap positions
	for (int x = 0;  x<= this->twiceEditDistanceThreshold; x++, mask= mask>>1) {
	  if (!(bitmap & mask)) {
	    editVector = editVector &this->maskEdit[x];
	  }
	}
	newEditVector = newEditVector |editVector;
	for(int x = 0; x< this->editDistanceThreshold;x++) {
	  newEditVector |= (newEditVector>>(this->editDistanceThreshold+2)& this->maskSum);
	}
	//  printf("newEditVector Final: %0lX\n", newEditVector);
	
	
      }
      //      printf("Edit Vector de Saida:");
      //	showEditVector(newEditVector); 
      return newEditVector;
    }
    
    inline unsigned isActive(int pos, unsigned long vet) {
      if (pos > this->editDistanceThreshold) return  0;
      pos+=this->editDistanceThreshold;
      if(vet& (0x8000000000000000 >> (((pos+1)*(this->editDistanceThreshold+1))-1))) {
	    return 1;
      }
      return 0;
    }
    
    void showEditVector(unsigned long vec) {
    unsigned cont = 0;
    int y;
    unsigned long masc;
    printf("( ");
    for(int x = 0 ; x < 2*this->editDistanceThreshold+1; x++) {
      masc=  0x8000000000000000>>(x*(this->editDistanceThreshold+1));
      for(y=0; ((masc & vec)==0) &&  y < this->editDistanceThreshold+1; y++) { masc>>=1; }
      
      printf("%d ",y);
    }
    printf(")\n");
  }


 void showEditVector2(unsigned long vec) {
    unsigned cont = 0;
    int y;
    unsigned long masc;
    //    printf("( ");
    for(int x = 1 ; x < 2*this->editDistanceThreshold; x++) {
      masc=  0x8000000000000000>>(x*(this->editDistanceThreshold+1));
      for(y=0; ((masc & vec)==0) &&  y < this->editDistanceThreshold+1; y++) { masc>>=1; }
      
      printf("%d\n",y);
    }
    //    printf(")\n");
  }

  
};

#endif //BEVA_BEVA_H
