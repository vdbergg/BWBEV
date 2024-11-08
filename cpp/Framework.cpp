//
// Created by vdberg on 12/02/19.
//

#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <sys/stat.h>
#include "../header/Trie.h"
#include "../header/C.h"
#include "../header/Framework.h"
#include "../header/utils.h"
#include "../header/Experiment.h"
#include "../header/Directives.h"

using namespace std;

Framework::Framework(unordered_map<string, string> config) {
    this->trie = nullptr;
    this->editDistanceThreshold = stoi(config["edit_distance"]);
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

    unsigned long fileSize = getFileSize(filename);
    char *tmpPtr = (char*) malloc(sizeof(char)*fileSize);
    StaticString::setDataBaseMemory(tmpPtr,fileSize);
    while (getline(input, str)) {
        if (!str.empty()) recs.push_back(StaticString(str));
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

    int queriesSize = stoi(config["queries_size"]);
    string datasetSuffix = queriesSize == 10 ? "_10" : "";
    string tau = to_string(this->editDistanceThreshold);

    switch (this->dataset) {
        case C::AOL:
            datasetFile += "aol/aol" + sizeSuffix + ".txt";
            queryFile += "aol/q17_" + tau + datasetSuffix + ".txt";
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
            break;
        default:
            datasetFile += "aol/aol" + sizeSuffix + ".txt";
            queryFile += "aol/q17_" + tau + datasetSuffix + ".txt";
            break;
    }

    readData(datasetFile, records);
    //    sort(this->records.begin(), this->records.end());
    readData(queryFile, this->queries);
    if (this->config["has_relevant_queries"] == "1") {
        readData(relevantQueryFile, this->relevantQueries);
    }

    this->trie = new Trie(this->experiment);
    this->trie->buildDaatIndex();
    this->trie->shrinkToFit();

    this->beva = new Beva(this->trie, this->experiment, this->editDistanceThreshold);

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
            vector<char *> results = output(currentActiveNodes);
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
