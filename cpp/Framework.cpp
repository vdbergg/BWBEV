//
// Created by vdberg on 12/02/19.
//

#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <sys/stat.h>
#include <time.h>
#include <stdlib.h>
#include "../header/Trie.h"
#include "../header/C.h"
#include "../header/Framework.h"
#include "../header/utils.h"
#include "../header/Experiment.h"
#include "../header/Directives.h"
#include "../header/TopKNode.h"

using namespace std;

Framework::Framework(unordered_map<string, string> config) {
    this->trie = nullptr;
    this->editDistanceThreshold = stoi(config["edit_distance"]);
    this->kResults = stoi(config["k_results"]);
    this->dataset = stoi(config["dataset"]);
    this->experiment = new Experiment(config, editDistanceThreshold);
    this->config = config;

    index();
}

Framework::~Framework() {
    cout << "deleting framework" << endl;
    delete this->beva;
    delete this->trie;
    delete this->experiment;
}

unsigned long getFileSize(string filename) {
    FILE *fp=fopen(filename.c_str(),"r");

    struct stat buf;
    fstat(fileno(fp), &buf);
    fclose(fp);
    return buf.st_size;
}

void Framework::readData(string& filename, vector<StaticString>& recs) {
    cout << "reading dataset " << filename << endl;

    string str;
    ifstream input(filename, ios::in);
    srand((unsigned) time(NULL));

    unsigned long fileSize = getFileSize(filename);
    char *tmpPtr = (char*) malloc(sizeof(char)*fileSize);
    StaticString::setDataBaseMemory(tmpPtr,fileSize);
    while (getline(input, str)) {
        if (!str.empty()) {
            recs.push_back(StaticString(str));
            float score = (float) rand() / RAND_MAX;
            scores.push_back(score);
        }
    }
}

void Framework::readData(string& filename, vector<string>& recs) {
    cout << "reading dataset " << filename << endl;

    string str;
    ifstream input(filename, ios::in);
    while (getline(input, str)) {
        if (!str.empty()) recs.push_back(str);
    }
}

void Framework::readData(string& filename, vector<double>& recs) {
    cout << "reading score dataset " << filename << endl;

    string str;
    ifstream input(filename, ios::in);
    while (getline(input, str)) {
        if (!str.empty()) recs.push_back(stod(str));
    }
}

void Framework::index(){
    cout << "indexing... \n";
    string sizeSuffix = "";
    switch (stoi(this->config["size_type"])) {
        case 0:
            sizeSuffix = "_20";
            break;
        case 1:
            sizeSuffix = "_40";
            break;
        case 2:
            sizeSuffix = "_60";
            break;
        case 3:
            sizeSuffix = "_80";
            break;
        case 4:
            sizeSuffix = "";
            break;
        default:
            sizeSuffix = "";
    }

    auto start = chrono::high_resolution_clock::now();

    #ifdef BEVA_IS_COLLECT_TIME_H
        this->experiment->initIndexingTime();
    #endif
    
    string datasetFile = this->config["dataset_basepath"];
    string queryFile = this->config["query_basepath"];
    string relevantQueryFile = this->config["query_basepath"];
    string scoreFile = config["score_basepath"];

    int queriesSize = stoi(config["queries_size"]);
    string datasetSuffix = queriesSize == 10 ? "_10" : "";
    string tau = to_string(this->editDistanceThreshold);

    switch (this->dataset) {
        case C::AOL:
            datasetFile += "aol/aol" + sizeSuffix + ".txt";
            queryFile += "aol/q17_" + tau + datasetSuffix + ".txt";
            scoreFile += "jusbrasil/scores.txt";
            break;
        case C::MEDLINE:
            datasetFile += "medline/medline" + sizeSuffix + ".txt";
            queryFile += "medline/q13" + datasetSuffix + ".txt";
            break;
        case C::USADDR:
            datasetFile += "usaddr/usaddr" + sizeSuffix + ".txt";
            queryFile += "usaddr/q17_" + tau + datasetSuffix + ".txt";
            break;
        case C::MEDLINE19:
            datasetFile += "medline19/medline19" + sizeSuffix + ".txt";
            queryFile += "medline19/q17_" + tau + datasetSuffix + ".txt";
            break;
        case C::DBLP:
            datasetFile += "dblp/dblp" + sizeSuffix + ".txt";
            queryFile += "dblp/q17_" + tau + datasetSuffix + ".txt";
	          break;
        case C::UMBC:
            datasetFile += "umbc/umbc" + sizeSuffix + ".txt";
            queryFile += "umbc/q17_" + tau + datasetSuffix + ".txt";
            break;
        case C::JUSBRASIL:
            datasetFile += "jusbrasil/jusbrasil" + sizeSuffix + ".txt";
            queryFile += "jusbrasil/q.txt";
            relevantQueryFile += "jusbrasil/relevant_answers.txt";
            scoreFile += "jusbrasil/scores.txt";
            break;
        default:
            datasetFile += "aol/aol" + sizeSuffix + ".txt";
            queryFile += "aol/q17_" + tau + datasetSuffix + ".txt";
            scoreFile += "jusbrasil/scores.txt";
            break;
    }

    readData(datasetFile, records);
    //    sort(this->records.begin(), this->records.end());
    readData(queryFile, this->queries);
    if (config["use_top_k_v1"] == "1" || config["use_top_k_v2"] == "1" || config["use_top_k_v3"] == "1") {
        readData(scoreFile, scores);
    }
    if (this->config["has_relevant_queries"] == "1") {
        readData(relevantQueryFile, this->relevantQueries);
    }

    this->trie = new Trie(this->experiment);
    this->trie->buildDaatIndex();
    this->trie->shrinkToFit();
    if (this->config["use_top_k_v2"] == "1" || this->config["use_top_k_v3"] == "1") {
        this->trie->buildMaxScores();
    }
    long long *preCalculatedExponentiation = new long long[this->editDistanceThreshold + 1];
    for (int i = 0; i <= this->editDistanceThreshold; i++) {
        preCalculatedExponentiation[i] = utils::fast_exponentiation(5, (this->editDistanceThreshold - i));
    }
    this->beva = new Beva(this->trie, this->experiment, this->editDistanceThreshold, preCalculatedExponentiation);
    if (config["use_top_k_v3"] == "1") {
        for (int i = 1; i <= this->editDistanceThreshold; i++) {
            preCalculatedExponentiation = new long long[i + 1];
            for (int j = 0; j <= i; j++) {
                preCalculatedExponentiation[j] = utils::fast_exponentiation(5, (i - j));
            }
            this->bevaTopK.push_back(new Beva(this->trie, experiment, i, preCalculatedExponentiation));
        }
    }

    auto done = chrono::high_resolution_clock::now();

    #ifdef BEVA_IS_COLLECT_MEMORY_H
        this->experiment->getMemoryUsedInIndexing();
    #else
        this->experiment->endIndexingTime();
        this->experiment->compileProportionOfBranchingSizeInBEVA2Level();
        this->experiment->compileNumberOfNodes();
    #endif
    cout << "<<<Index time: "<< chrono::duration_cast<chrono::milliseconds>(done - start).count() << " ms>>>\n";
}

void Framework::processQueryWithTopKBruteForce(string &query, int queryId) {
    vector<ActiveNode> currentActiveNodes;
    vector<ActiveNode> oldActiveNodes;

    unsigned bitmaps[CHAR_SIZE];
    for (auto & bitmap : bitmaps) bitmap = this->beva->bitmapZero;

    for (int currentPrefixQuery = 1; currentPrefixQuery <= query.size(); currentPrefixQuery++) {
        swap(oldActiveNodes, currentActiveNodes);
        currentActiveNodes.clear();

        #ifdef BEVA_IS_COLLECT_TIME_H
        experiment->initQueryProcessingTime();
        #endif

        this->beva->process(query[currentPrefixQuery - 1],
                            currentPrefixQuery,
                            oldActiveNodes,
                            currentActiveNodes,
                            bitmaps);

        #ifdef BEVA_IS_COLLECT_TIME_H
        experiment->endQueryProcessingTime(currentActiveNodes.size(), currentPrefixQuery);

        vector<int> prefixesSize = { 5, 9, 13, 17 };
        if (std::find(prefixesSize.begin(), prefixesSize.end(), currentPrefixQuery) != prefixesSize.end()) {
            experiment->initQueryFetchingTime();
            TopKHeap topKHeap(this->kResults);
            buildTopKBruteForce(currentActiveNodes, currentPrefixQuery, topKHeap);
            vector<char *> results = topKHeap.outputSuggestions();
            experiment->endQueryFetchingTime(currentPrefixQuery, results.size());
        }
        #endif

        currentActiveNodes.shrink_to_fit();
        if (query.length() == currentPrefixQuery) {
            #ifdef BEVA_IS_COLLECT_MEMORY_H
            this->experiment->getMemoryUsedInProcessing();
            #else
            experiment->compileQueryProcessingTimes(queryId);
            string currentQuery = query.substr(0, currentPrefixQuery);
            experiment->saveQueryProcessingTime(currentQuery, queryId);
            #endif
        }

        oldActiveNodes.clear();
    }
}

void Framework::buildTopKBruteForce(vector<ActiveNode>& currentActiveNodes,
                                    double querySize,
                                    TopKHeap& topKHeap) const {
    for (ActiveNode activeNode : currentActiveNodes) {
        if (activeNode.editDistance == -1) {
            int pos = ((int) querySize - (int) activeNode.level) + this->editDistanceThreshold;
            activeNode.editDistance = this->beva->retrieveEditDistance(pos, activeNode.editVector);
        }

        unsigned beginRange = this->trie->getNode(activeNode.node).getBeginRange();
        unsigned endRange = this->trie->getNode(activeNode.node).getEndRange();
        long long score = this->beva->preCalculatedExponentiation[activeNode.editDistance];

        for (unsigned i = beginRange; i < endRange; i++) {
            double dynamicScore = utils::dynamicScore(scores[i], score);
            TopKNode nodeToInsert(i, dynamicScore);
            topKHeap.insertNode(nodeToInsert);
        }
    }
}

void Framework::processQueryWithTopKPruningV1(string &query, int queryId) {
    vector<ActiveNode> currentActiveNodes;
    vector<ActiveNode> oldActiveNodes;

    unsigned bitmaps[CHAR_SIZE];
    for (auto & bitmap : bitmaps) bitmap = this->beva->bitmapZero;

    for (int currentPrefixQuery = 1; currentPrefixQuery <= query.size(); currentPrefixQuery++) {
        swap(oldActiveNodes, currentActiveNodes);
        currentActiveNodes.clear();

        #ifdef BEVA_IS_COLLECT_TIME_H
        experiment->initQueryProcessingTime();
        #endif

        this->beva->process(query[currentPrefixQuery - 1],
                            currentPrefixQuery,
                            oldActiveNodes,
                            currentActiveNodes,
                            bitmaps);

        #ifdef BEVA_IS_COLLECT_TIME_H
        experiment->endQueryProcessingTime(currentActiveNodes.size(), currentPrefixQuery);

        vector<int> prefixesSize = { 5, 9, 13, 17 };
        if (std::find(prefixesSize.begin(), prefixesSize.end(), currentPrefixQuery) != prefixesSize.end()) {
            experiment->initQueryFetchingTime();
            TopKHeap topKHeap(this->kResults);
            buildTopKWithPruningV1Range(currentActiveNodes, currentPrefixQuery, topKHeap);
            vector<char *> results = topKHeap.outputSuggestions();
            experiment->endQueryFetchingTime(currentPrefixQuery, results.size());
        }
        #endif

        currentActiveNodes.shrink_to_fit();
        if (query.length() == currentPrefixQuery) {
            #ifdef BEVA_IS_COLLECT_MEMORY_H
            this->experiment->getMemoryUsedInProcessing();
            #else
            experiment->compileQueryProcessingTimes(queryId);
            string currentQuery = query.substr(0, currentPrefixQuery);
            experiment->saveQueryProcessingTime(currentQuery, queryId);
            #endif
        }

        oldActiveNodes.clear();
    }
}

void Framework::buildTopKWithPruningV1Range(vector<ActiveNode>& currentActiveNodes,
                                            double querySize,
                                            TopKHeap& topKHeap) const {
    for (ActiveNode activeNode : currentActiveNodes) {
        double activeNodeScore = this->trie->getNode(activeNode.node).getMaxStaticScore();
        unsigned recordIdFromActiveNodeScore = this->trie->getNode(activeNode.node).getRecordIdFromMaxScore();

        if (topKHeap.isFull() && (activeNodeScore < topKHeap.topMaxScore() || topKHeap.contains(recordIdFromActiveNodeScore))) continue;

        if (activeNode.editDistance == -1) {
            int pos = ((int) querySize - (int) activeNode.level) + this->editDistanceThreshold;
            activeNode.editDistance = this->beva->retrieveEditDistance(pos, activeNode.editVector);
        }

        unsigned beginRange = this->trie->getNode(activeNode.node).getBeginRange();
        unsigned endRange = this->trie->getNode(activeNode.node).getEndRange();
        long long score = this->beva->preCalculatedExponentiation[activeNode.editDistance];

        for (unsigned i = beginRange; i < endRange; i++) {
            double dynamicScore = utils::dynamicScore(scores[i],
                                                      score);
            TopKNode nodeToInsert(i, dynamicScore);
            topKHeap.insertNode(nodeToInsert);
        }
    }
}

void Framework::processQueryWithTopKPruningV2(string &query, int queryId) {
    vector<vector<ActiveNode>> currentActiveNodes;
    vector<vector<ActiveNode>> oldActiveNodes;
    unsigned bitmaps[4][CHAR_SIZE];

    for (int i = 0; i <= this->editDistanceThreshold; i++) {
        currentActiveNodes.emplace_back(vector<ActiveNode>());
        oldActiveNodes.emplace_back(vector<ActiveNode>());

        if (i > 0) {
            for (auto &bitmap : bitmaps[i]) {
                bitmap = this->bevaTopK[i - 1]->bitmapZero;
            }
        }
    }
    oldActiveNodes[0].emplace_back(this->trie->root, this->beva->editVectorStartValue, 0, 0);

    TopKHeap topKHeap(this->kResults);
    for (int currentPrefixQuery = 1; currentPrefixQuery <= query.size(); currentPrefixQuery++) {
        swap(oldActiveNodes, currentActiveNodes);
        for (int i = 0; i <= this->editDistanceThreshold; i++) {
            currentActiveNodes[i].clear();
        }

        #ifdef BEVA_IS_COLLECT_TIME_H
        experiment->initQueryProcessingTime();
        #endif

        this->beva->processNoErrors(query[currentPrefixQuery - 1],
                                    oldActiveNodes[0],
                                    currentActiveNodes[0],
                                    topKHeap);

        for (int i = 1; i <= this->editDistanceThreshold; i++) {
            this->bevaTopK[i - 1]->processWithPruningV2(query[currentPrefixQuery - 1],
                                                        currentPrefixQuery,
                                                        oldActiveNodes[i],
                                                        currentActiveNodes[i],
                                                        bitmaps[i],
                                                        topKHeap,
                                                        i);
        }

        #ifdef BEVA_IS_COLLECT_TIME_H
        int numberOfActiveNodes = 0;
        for (vector<ActiveNode>& activeNodes : currentActiveNodes) {
            numberOfActiveNodes += activeNodes.size();
        }
        experiment->endQueryProcessingTime(numberOfActiveNodes, currentPrefixQuery);

        vector<int> prefixesSize = { 5, 9, 13, 17 };
        if (std::find(prefixesSize.begin(), prefixesSize.end(), currentPrefixQuery) != prefixesSize.end()) {
            experiment->initQueryFetchingTime();

            buildTopKWithPruningV2Range(currentActiveNodes[0],
                                        currentPrefixQuery,
                                        this->beva->preCalculatedExponentiation,
                                        topKHeap,
                                        0);

            for (int i = 1; i <= this->editDistanceThreshold; i++) {
                buildTopKWithPruningV2Range(currentActiveNodes[i],
                                            currentPrefixQuery,
                                            this->bevaTopK[i - 1]->preCalculatedExponentiation,
                                            topKHeap,
                                            i);
            }

            vector<char *> results = topKHeap.outputSuggestions();
            experiment->endQueryFetchingTime(currentPrefixQuery, results.size());
        }
        #endif

        currentActiveNodes.shrink_to_fit();
        if (query.length() == currentPrefixQuery) {
            #ifdef BEVA_IS_COLLECT_MEMORY_H
            this->experiment->getMemoryUsedInProcessing();
            #else
            experiment->compileQueryProcessingTimes(queryId);
            string currentQuery = query.substr(0, currentPrefixQuery);
            experiment->saveQueryProcessingTime(currentQuery, queryId);
            #endif
        }

        for (int i = 0; i <= this->editDistanceThreshold; i++) {
            oldActiveNodes[i].clear();
        }
    }
}

void Framework::buildTopKWithPruningV2Range(vector<ActiveNode>& currentActiveNodes,
                                            double querySize,
                                            const long long* preCalculatedExponentiation,
                                            TopKHeap& topKHeap,
                                            int currentEditDistance) const {
    for (ActiveNode activeNode : currentActiveNodes) {
        if (activeNode.editDistance == -1) {
            int pos = ((int) querySize - (int) activeNode.level) + currentEditDistance;
            activeNode.editDistance = this->beva->retrieveEditDistance(pos, activeNode.editVector);
        }

        if (activeNode.editDistance == currentEditDistance) {
            double activeNodeScore = this->trie->getNode(activeNode.node).getMaxStaticScore();
            unsigned recordIdFromActiveNodeScore = this->trie->getNode(activeNode.node).getRecordIdFromMaxScore();

            if (topKHeap.isFull() && (activeNodeScore < topKHeap.topMaxScore() || topKHeap.contains(recordIdFromActiveNodeScore))) continue;


            unsigned beginRange = this->trie->getNode(activeNode.node).getBeginRange();
            unsigned endRange = this->trie->getNode(activeNode.node).getEndRange();
            long long score = preCalculatedExponentiation[activeNode.editDistance];

            for (unsigned i = beginRange; i < endRange; i++) {
                double dynamicScore = utils::dynamicScore(scores[i], score);
                TopKNode nodeToInsert(i, dynamicScore);
                topKHeap.insertNode(nodeToInsert);
            }
        }
    }
}

vector<char *> Framework::processFullQuery(string &query, int queryPosition) {
    #ifdef BEVA_IS_COLLECT_TIME_H
        this->experiment->initQueryProcessingTime();
    #endif

    vector<ActiveNode> currentActiveNodes;
    vector<ActiveNode> oldActiveNodes;
    
    unsigned *bitmaps= new unsigned [CHAR_SIZE];
    for (unsigned x = 0; x < CHAR_SIZE; x++) bitmaps[x] = 0;

    for (int currentPrefixQuery = 1; currentPrefixQuery <= query.size(); currentPrefixQuery++) {
        swap(oldActiveNodes, currentActiveNodes);
        currentActiveNodes.clear();
        this->beva->process(query[currentPrefixQuery - 1], currentPrefixQuery, oldActiveNodes,
			    currentActiveNodes, bitmaps);
        currentActiveNodes.shrink_to_fit();
        oldActiveNodes.clear();
    }

    #ifdef BEVA_IS_COLLECT_TIME_H
        this->experiment->endSimpleQueryProcessingTime(currentActiveNodes.size());
        this->experiment->initQueryFetchingTime();
    #endif
    vector<char *> results = this->output(currentActiveNodes);

    #ifdef BEVA_IS_COLLECT_TIME_H
        this->experiment->endSimpleQueryFetchingTime(results.size());

        bool relevantReturned = false;
        if (queryPosition != -1 && this->config["has_relevant_queries"] == "1") {
            vector<string> v_output;
            v_output.resize(results.size());
            copy(results.begin(), results.end(), v_output.begin());

            relevantReturned = find(v_output.begin(), v_output.end(),
                    this->relevantQueries[queryPosition]) != v_output.end();
        }
        this->experiment->compileSimpleQueryProcessingTimes(query, relevantReturned);
    #endif

    #ifdef BEVA_IS_COLLECT_MEMORY_H
        this->experiment->getMemoryUsedInProcessing();
    #endif
    delete[] bitmaps;
    return results;
}

vector<char *> Framework::processQuery(string &query, int queryId) {
    vector<ActiveNode> currentActiveNodes;
    vector<ActiveNode> oldActiveNodes;

    unsigned *bitmaps = new unsigned[CHAR_SIZE];
    for (unsigned x = 0; x < CHAR_SIZE; x++) bitmaps[x] = 0;

    for (int currentPrefixQuery = 1; currentPrefixQuery <= query.size(); currentPrefixQuery++) {
      swap(oldActiveNodes, currentActiveNodes);
      currentActiveNodes.clear();
      this->process(query, currentPrefixQuery, queryId, oldActiveNodes, currentActiveNodes, bitmaps);
      oldActiveNodes.clear();
    }
    delete[] bitmaps;
    vector<char *> results = this->output(currentActiveNodes);
    return results;
}

void Framework::process(string query, int prefixQueryLength, int currentCountQuery,
			vector<ActiveNode>& oldActiveNodes, vector<ActiveNode>& currentActiveNodes, unsigned *bitmaps) {
    if (query.empty()) return;
   
    #ifdef BEVA_IS_COLLECT_TIME_H
        this->experiment->initQueryProcessingTime();
    #endif

    this->beva->process(query[prefixQueryLength - 1], prefixQueryLength, oldActiveNodes, currentActiveNodes, bitmaps);

    #ifdef BEVA_IS_COLLECT_TIME_H
        this->experiment->endQueryProcessingTime(currentActiveNodes.size(), prefixQueryLength);

        vector<int> prefixQuerySizeToFetching = { 5, 9, 13, 17 };
        if (std::find(prefixQuerySizeToFetching.begin(), prefixQuerySizeToFetching.end(), prefixQueryLength) !=
            prefixQuerySizeToFetching.end()) {
            this->experiment->initQueryFetchingTime();
            vector<char *> results = this->output(currentActiveNodes);
            this->experiment->endQueryFetchingTime(prefixQueryLength, results.size());
        }
    #endif

    currentActiveNodes.shrink_to_fit();
    if (query.length() == prefixQueryLength) {
        #ifdef BEVA_IS_COLLECT_MEMORY_H
            this->experiment->getMemoryUsedInProcessing();
        #else
            this->experiment->compileQueryProcessingTimes(currentCountQuery);
            string currentQuery = query.substr(0, prefixQueryLength);
            this->experiment->saveQueryProcessingTime(currentQuery, currentCountQuery);
        #endif
    }
}

void Framework::writeExperiments() {
    #ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
        this->experiment->compileNumberOfActiveNodes();
        this->experiment->compileNumberOfIterationInChildren();
    #endif
}

vector<char *> Framework::output(vector<ActiveNode>& currentActiveNodes) {
    vector<char *> outputs;
    string tmp;
    // int limit = 100;

    for (ActiveNode activeNode : currentActiveNodes) {
        unsigned beginRange = this->trie->getNode(activeNode.node).getBeginRange();
        unsigned endRange = this->trie->getNode(activeNode.node).getEndRange();

        for (unsigned i = beginRange; i < endRange; i++) {
            outputs.push_back(records[i].c_str());
	    // if (outputs.size() >= limit) return outputs;
        }
    }
  
    //    for (const string& record : outputs) {
    //     cout << record << "\n";
    // }
//    cout << "Número de resultados: " << outputs.size() << endl;
    return outputs;
}
