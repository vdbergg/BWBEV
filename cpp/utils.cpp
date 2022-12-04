//
// Created by berg on 03/02/19.
//

#include <iostream>
#include <sstream>
#include <string>
#include "../header/utils.h"
#include "../header/Beva.h"

using namespace std;


void utils::split(string value, char delimiter, vector<string>& tokens) {
    stringstream check1(value);
    string intermediate;

    while(getline(check1, intermediate, delimiter)) {
        tokens.push_back(intermediate);
    }
}

void utils::printVector(int* vector, int size) {
    for(int i = 0; i < size; ++i) {
        cout << vector[i] << " ";
    }
    cout << "\n";
}

void utils::printTransition(int* from, int sizeFrom, int* to, int sizeTo, string& bitmap) {
    cout << "f([ ";
    for(int i = 0; i < sizeFrom; ++i) {
        cout << from[i] << " ";
    }
    cout << "], " + bitmap + ") = [ ";
    for(int i = 0; i < sizeTo; ++i) {
        cout << to[i] << " ";
    }
    cout << "]\n";
}

string utils::shiftBitInBinaryString(string& bitmap, int countShiftBit, int bitmapSize) {
    int decimal = (int) bitset<16>(bitmap).to_ulong();
    decimal = decimal << countShiftBit; // shifting 1 bit leftwards
    bitmap = bitset<16>(decimal).to_string();
    bitmap = bitmap.substr(bitmap.length() - bitmapSize);

    return bitmap;
}

string utils::convertDecimalToBinaryString(int decimal, int bitmapSize) {
    string bitmap = bitset<16>(decimal).to_string();
    return bitmap.substr(bitmap.length() - bitmapSize);
}

char utils::convertSpecialCharToSimpleChar(char c) {
    switch ((int) c) {
        case -92: return 'a';
        case -93: return 'a';
        case -94: return 'a';
        case -95: return 'a';
        case -96: return 'a';
        case -124: return 'a';
        case -125: return 'a';
        case -126: return 'a';
        case -127: return 'a';
        case -128: return 'a';
        case -85: return 'e';
        case -86: return 'e';
        case -87: return 'e';
        case -88: return 'e';
        case -117: return 'e';
        case -118: return 'e';
        case -119: return 'e';
        case -120: return 'e';
        case -81: return 'i';
        case -82: return 'i';
        case -83: return 'i';
        case -84: return 'i';
        case -113: return 'i';
        case -114: return 'i';
        case -115: return 'i';
        case -116: return 'i';
        case -74: return 'o';
        case -75: return 'o';
        case -76: return 'o';
        case -77: return 'o';
        case -78: return 'o';
        case -106: return 'o';
        case -107: return 'o';
        case -108: return 'o';
        case -109: return 'o';
        case -110: return 'o';
        case -68: return 'u';
        case -69: return 'u';
        case -70: return 'u';
        case -71: return 'u';
        case -100: return 'u';
        case -101: return 'u';
        case -102: return 'u';
        case -103: return 'u';
        case -89: return 'c';
        case -121: return 'c';
        case -79: return 'n';
        case -111: return 'n';
        default: return '?'; // unknown
    }
}

string utils::normalize(string& data) {
    string normalized = "";
    for (char &c : data) {
        if ((int) c == -61) continue;
        else if ((int) c < 0 || (int) c >= CHAR_SIZE) {
            c = utils::convertSpecialCharToSimpleChar(c);
        }
        normalized += c;
    }
    data = normalized;
    return data;
}
