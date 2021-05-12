//
// Created by berg on 28/01/19.
//

#include "../header/State.h"
#include <utility>

State::State(EditVector *editVector, int id, bool isInitial, bool isFinal) {
    this->editVector = editVector;
    this->isInitial = isInitial;
    this->isFinal = isFinal;
    this->id = id;
//    for (int i = 0; i < this->editVector->size; i++) {
//        this->value += to_string(this->editVector->vector[i]);
//    }
}

State::~State() {
    delete this->editVector;
};
