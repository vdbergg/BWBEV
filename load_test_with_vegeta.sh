#!/usr/bin/env bash

requests_per_second="50 100 150 200 210 220 230 240 260 270 280 290 300 310 320 330 340 350 360 370 380 390 400 410 420 430 440 450 460 470 480 490 500 510 520 530 540 550 560 570 580 590 600"
dataset=$1
method="bshift"
base_path="/home/bdri-berg/storage/datasets/autocompletion/${dataset}/q17_3_load_test_vegeta.txt"

for r in ${requests_per_second}
do
  file_name_bin="${method}-r${r}s-${dataset}.bin"
  file_name_json="${method}-r${r}s-${dataset}.json"
  executable_path="vegeta attack -duration=30s -rate=${r}/s --targets=${base_path} -output=${method}-r${r}s-${dataset}.bin"
  ${executable_path} && cat ${file_name_bin} | vegeta report --type=json > ${file_name_json}
done

