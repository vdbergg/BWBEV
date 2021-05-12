//
// Created by berg on 28/01/19.
//

#ifndef BEVA_EDITVECTORAUTOMATON_H
#define BEVA_EDITVECTORAUTOMATON_H

#include <unordered_map>
#include "State.h"

class EditVectorAutomata {
public:
    State* initialState;
    State* finalState;
    int editDistanceThreshold;
    int size;
    int transitionsByState;

    EditVectorAutomata(int);

    ~EditVectorAutomata();

    State* setTransition(State*&, unsigned, unordered_map<VectorChar, State*, MyHashVectorFunction>&);
    void buildAutomaton();
};

#endif //BEVA_EDITVECTORAUTOMATON_H
