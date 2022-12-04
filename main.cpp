#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "header/Framework.h"
#include "header/Directives.h"
#ifndef BEVA_IS_MAC_H
#include "header/crow_all.h"
#endif

using namespace std;

unordered_map<string, string> config;
void loadConfig();
Framework* framework;


void processingQueriesOutsideServer() {
    cout << "processing...\n";

    int indexMin = stoi(config["qry_number_start"]);
    int indexMax =  stoi(config["qry_number_end"]);

    #ifdef BEVA_IS_COLLECT_MEMORY_H
        indexMax = 100;
    #endif

    if (config["use_top_k_v1"] == "1") {
        for (int i = indexMin; i < indexMax; ++i) {
            framework->processQueryWithTopKBruteForce(framework->queries[i], i);
        }
    } else if (config["use_top_k_v2"] == "1") {
        for (int i = indexMin; i < indexMax; ++i) {
            framework->processQueryWithTopKPruningV1(framework->queries[i], i);
        }
    } else if (config["use_top_k_v3"] == "1") {
        for (int i = indexMin; i < indexMax; ++i) {
            framework->processQueryWithTopKPruningV2(framework->queries[i], i);
        }
    } else if (config["is_full_query_instrumentation"] == "0") {
        for (int i = indexMin; i < indexMax; ++i) {
            framework->processQuery(framework->queries[i], i);
        }
    } else {
        for (int i = indexMin; i < indexMax; ++i) {
            framework->processFullQuery(framework->queries[i], i);
        }
    }
}

void processingQueriesInServer() {
    #ifndef BEVA_IS_MAC_H
    crow::SimpleApp app;

    CROW_ROUTE(app, "/autocomplete")
            ([](const crow::request& req) {
                std::ostringstream os;
                vector<char *> results;

                os << "Params: " << req.url_params << "\n\n";

                if (req.url_params.get("query") != nullptr) {
                    string query = boost::lexical_cast<string>(req.url_params.get("query"));
                    os << "The value of 'query' is " <<  query << '\n';
                    results = framework->processFullQuery(query);
                }

                crow::json::wvalue response;
                response["results"] = results;
                cout << "<<<< Response length >>>> " + to_string(results.size()) << endl;

                return response;
//                return crow::response{os.str()};
            });

    // ignore all log
    crow::logger::setLogLevel(crow::LogLevel::Debug);
    //crow::logger::setHandler(std::make_shared<ExampleLogHandler>());

    app.port(18080)
//      .concurrency(10)
        .multithreaded()
        .run();
    #endif
}


int main(int argc, char** argv) {
    loadConfig();

    framework = new Framework(config);

    if (config["is_server"] == "0") {
        processingQueriesOutsideServer();
    } else {
        processingQueriesInServer();
    }

    #ifdef BEVA_IS_COLLECT_COUNT_OPERATIONS_H
        framework->writeExperiments();
    #endif

    delete framework;
    return 0;
}

void loadConfig() {
    ifstream is_file("./path.cfg");
    string line;

    while (std::getline(is_file, line)) {
        istringstream is_line(line);
        string key;

        if (getline(is_line, key, '=')) {
            string value;

            if (getline(is_line, value)) {
                config[key] = value;
            }
        }
    }
}
