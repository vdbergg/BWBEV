//
// Created by berg on 13/01/19.
//

#include "../header/Trie.h"
#include "../header/Experiment.h"
#include "../header/Directives.h"

vector<StaticString> records;
vector<double> scores;

Trie::Trie(Experiment* experiment) {
    this->experiment = experiment;
    if (this->experiment->config["dataset"] == "3") { // Medline
        this->globalMemory.reserve(2308200000);
    } else {
        this->globalMemory.reserve(records.size() * 4);
    }

    this->root = newNode();
    getNode(this->root).setBeginRange(0);
    getNode(this->root).setEndRange(records.size());
    this->experiment->incrementNumberOfNodes();

    this->lastNodeKnownPerRecord.reserve(records.size());
    for (int recordId = 0; recordId < records.size(); recordId++) {
        this->lastNodeKnownPerRecord[recordId] = this->root;
    }
}

void Trie::buildDaatIndex() {
    int maxLevel = records[0].length();

    for (int currentIndexLevel = 0; currentIndexLevel < maxLevel; currentIndexLevel++) {
        for (int recordId = 0; recordId < records.size(); recordId++) {

            if (currentIndexLevel <= records[recordId].length() - 1) {
                unsigned parent = this->lastNodeKnownPerRecord[recordId];

                if (records[recordId].length() > maxLevel) {
                    maxLevel = records[recordId].length();
                }

                unsigned char ch = records[recordId][currentIndexLevel];
                unsigned node = this->insert((char) ch, recordId, parent);
                getNode(node).setEndRange(recordId + 1);
                this->lastNodeKnownPerRecord[recordId] = node;

                if (currentIndexLevel == records[recordId].length() - 1) {
                    getNode(node).setIsEndOfWord(true);
                    if (this->experiment->config["use_top_k_v1"] == "1" ||
                        this->experiment->config["use_top_k_v2"] == "1" ||
                        this->experiment->config["use_top_k_v3"] == "1" ||
                        this->experiment->config["use_top_k_v4"] == "1") {
                        getNode(node).setMaxStaticScore(scores[recordId]);
                        getNode(node).setRecordIdFromMaxScore(recordId);
                    }
                    #ifdef BEVA_IS_COLLECT_TIME_H
                        this->experiment->proportionOfBranchingSize(currentIndexLevel + 1);
                    #endif
                }
            }
        }
    }
}

unsigned Trie::insert(char ch, int recordId, unsigned parent) {
    unsigned child;
    unsigned numChildren = getNode(parent).numChildren;
    if (numChildren) {
        unsigned child = getNode(parent).children + numChildren - 1;
        if (getNode(child).getValue() == ch) return child;
    }
    unsigned node = newNode();
    if (numChildren == 0) getNode(parent).children = node;

    getNode(node).setValue(ch);
    getNode(node).setBeginRange(recordId);
    getNode(parent).numChildren++;
    #ifdef BEVA_IS_COLLECT_TIME_H
    this->experiment->incrementNumberOfNodes();
    #endif

    return node;
}

int Trie::search(char ch, unsigned parent) {
    unsigned numChildren = getNode(parent).numChildren;

    if (numChildren) {
        unsigned firstChild = getNode(parent).children;
        unsigned lastChild = firstChild + getNode(parent).numChildren;

        for (unsigned node = firstChild; node < lastChild; node++) {
            if (getNode(node).getValue() == ch) return node;
        }
    }

    return -1;
}

void Trie::buildMaxScores() {
    for (int recordId = 0; recordId < records.size(); recordId++) {
        string record = records[recordId].c_str();
        unsigned parent = this->root;

        for (char ch : record) {
            unsigned node = this->search(ch, parent);
            if (node == -1) exit(1);

            if (scores[recordId] > getNode(node).maxStaticScore) {
                getNode(node).setMaxStaticScore(scores[recordId]);
                getNode(node).setRecordIdFromMaxScore(recordId);
            }

            parent = node;
        }
    }

    // atribuir o max_score do nó raiz
    unsigned firstChild = getNode(this->root).children;
    unsigned lastChild = firstChild + getNode(this->root).numChildren;

    for (unsigned i = firstChild; i < lastChild; i++) {
        if (getNode(this->root).getMaxStaticScore() < getNode(i).getMaxStaticScore()) {
            getNode(this->root).setMaxStaticScore(getNode(i).getMaxStaticScore());
            getNode(this->root).setRecordIdFromMaxScore(getNode(i).getRecordIdFromMaxScore());
        }
    }
}

void Trie::shrinkToFit() {
    this->globalMemory.shrink_to_fit();

    this->lastNodeKnownPerRecord.clear();
    this->lastNodeKnownPerRecord.shrink_to_fit();
}
