//
// Created by berg on 03/02/19.
//

#ifndef BEVA_UTILS_H
#define BEVA_UTILS_H

#include <vector>
#include <bitset>
#include <iostream>

using namespace std;

class utils {
public:
    static void split(string, char, vector<string>&);
    static void printVector(int*, int);
    static void printTransition(int*, int, int*, int, string&);
    static string shiftBitInBinaryString(string&, int, int);
    static string convertDecimalToBinaryString(int, int);
    static char convertSpecialCharToSimpleChar(char);
    static string normalize(string&);

    static inline int min(int a, int b, int c) {
        return std::min(std::min(a, b), c);
    };
    static inline void printBitsFromDecimal(unsigned decimal) {
        bitset<16> bitset1{decimal};
        cout << bitset1 << endl;
    };
    static inline unsigned leftShiftBitInDecimal(unsigned decimal, unsigned countShiftBit, unsigned bitmapSize) {
        return (decimal << countShiftBit) & bitmapSize;
    };
    static inline unsigned char getKthBitFromDecimal(unsigned int decimal, unsigned k) {
        return (decimal >> k) & 1;
    };
    static inline unsigned int setKthBitFromDecimal(unsigned int decimal, unsigned short k, unsigned bitmapSize) {
        return (1 << k | decimal) & bitmapSize;
    };
};


#endif //BEVA_UTILS_H
