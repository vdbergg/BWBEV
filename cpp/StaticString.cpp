//
// Created by berg on 23/07/2020.
//

#include "../header/StaticString.h"

integerPointer currFreePosition;
integerPointer dataEnd;
char *_data_static;

bool operator<( const StaticString& x1, const StaticString& x2 ) {
    return strcmp(_data_static+x1.dataPos,_data_static+x2.dataPos) < 0;
}


bool operator==( const StaticString& x1, const StaticString& x2 ) {
    return strcmp(_data_static+x1.dataPos,_data_static+x2.dataPos) == 0;
}


