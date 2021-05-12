#!/usr/bin/env bash

#concurrencies="10 40 70 100 130 160 190 220 250"
concurrencies="10 20 40 60 80 100 200 400 600 800 1000"
#number_of_requests="500"
base_path="/mnt/storage/datasets/autocompletion/aol/q17_3_load_test.txt"

for concurrency in ${concurrencies}
do
#  executable_path="siege -f ${base_path} -d 1 -c ${concurrency} -r ${number_of_requests}"
  executable_path="siege -f ${base_path} -i -b -r 10 -c ${concurrency}"
  ${executable_path}
done
