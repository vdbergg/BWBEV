# BShift: A Bit Parallelism Approach for Approximate Prefix Search on Large Datasets

**Authors:** Edleno Silva de Moura · Berg Ferreira · Altigran da Silva · Ricardo Baeza-Yates

**Journal:** Very Large Data Base Endowment Inc. (VLDB Endowment)

**Abstract:** In this study, we address the issue of performing an approximate pre-
fix search on a set of strings. We discuss the use of a bit-parallelism
approach to calculate the edit distance between two distinct strings
and demonstrate how to adapt it for use in an approximate prefix
search method. We propose a trie-based method, named BShift, that
utilizes a unary representation of the edit vectors and bitwise oper-
ations to update them when calculating the edit distance between
strings. Our experimental results with the new method indicate
that it can significantly accelerate processing by more than 36%
when compared to state-of-the-art methods.

**DOI:** 

**Note:** For any reference to the repository, please cite this article.


## Requirements

1. Boost library
2. Pthread library

## How to use

This repository includes implementations of BShift method

### Running a method

For run a method follow the steps:

1. Compile the source code:

> g++ -std=c++11 -I. cpp/* *.cpp -O3 -o output -lboost_system -lpthread

2. Run the binary file using the path.cfg file of the method:

> ./output

The `path.cfg` is the file of config

## Parameters

* **<edit_distance_threshold>**: An integer from 1 to 5.

* **<<dataset_number>>**: An integer from 0 to 6, where

0. **AOL**
1. **MEDLINE**
2. **USADDR**
3. **MEDLINE19**
4. **DBLP**
5. **UMBC**
6. **JUSBRASIL**

* **<queries_size>:** An integer indicating the number of queries that will be read from queries file.


* **<qry_number_start>**: An integer indicating the start index in the queries file.


* **<qry_number_end>**: An integer indicating the end index in the queries file.


* **<size_type>**: An integer from 0 to 4 indicating the portion of the size of the dataset, where

0. **20%**
1. **40%**
2. **60%**
3. **80%**
4. **100%**

OBS: The portion of the dataset must be generated previously with the correspondent suffix as `aol_20.txt` or `aol.txt` to 100% of the AOL dataset.


* **<dataset_basepath>**: The string base path to the dataset. Into of the base path must have for each dataset a
  directory with the dataset name and into of this directory must have the dataset file with the name of the dataset and the
  extension .txt. For example `/mnt/storage/datasets/autocompletion/aol/aol.txt`, the `/mnt/storage/datasets/autocompletion/`
  is the base path, `aol` is the directory and `aol.txt` is the dataset file.


* **<query_basepath>**: The string base path to the queries. Into of the base path must have for each dataset a
  directory with the dataset name and into of this directory must have the queries file with the name `q.txt`.
  For example `/mnt/storage/datasets/autocompletion/aol/q.txt`, the `/mnt/storage/datasets/autocompletion/`
  is the base path, `aol` is the directory and `q17_1.txt` is the queries file from the aol dataset with 17 characters
  to edit distance threshold equal to 1.

* **<experiments_basepath>**: The string with the base path for save the experiments output.


* **<is_server>**: A bool with value 0 or 1, being 1 to the method up as a server, or 0 otherwise.


* **<is_full_query_instrumentation>**: A bool with value 0 or 1, being 1 to instrument the queries
  by full query or 0 to the instrumentation be performed character by character, simulating each user's keystroke.

## Datasets

* The datasets must be built with a one suggestion (set of words) by row.


* The queries must be built synthetically with a query by row extracted from dataset and introduced errors randomly.

**OBS: In this repo we provide the AOL dataset.**
