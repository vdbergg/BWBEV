#!/bin/bash

SERVICE=$1
project_path=$2
executable_path="$project_path/$SERVICE"
cfg_path="$project_path/path.cfg"
executable_run_script_bash_path="bash $project_path/run.sh $executable_path $cfg_path"
free_cache="echo 3 > /proc/sys/vm/drop_caches"

if [ ! -n "$SERVICE" ] 
then
	echo "$1 - Error \$1 not set or NULL. SERVICE"
	exit 1
fi

if [ ! -n "$executable_path" ] 
then
	echo "$2 - Error \$2 not set or NULL. executable_path"
	exit 1
fi

if [ ! -n "$cfg_path" ] 
then
	echo "$3 - Error \$3 not set or NULL. cfg_path"
	exit 1
fi

if pgrep -x "$SERVICE" >/dev/null
then
	echo "$SERVICE is running"
else
	echo "$SERVICE stopped"

	declare -a cfg_array

	IFS=$'\n' read -d '' -r -a lines_cfg_file < ${cfg_path}

	for i in "${lines_cfg_file[@]}"
	do
	    IFS='=' read -ra tokens <<< "$i"
		cfg_array+=(${tokens[1]})
	done

	edit_distance=${cfg_array[0]}
	dataset=${cfg_array[1]}
	queries_size=${cfg_array[2]}
	recovery_mode=${cfg_array[3]}
	qry_number_start=${cfg_array[4]}
	qry_number_end=${cfg_array[5]}
	size_type=${cfg_array[6]}
	alg=${cfg_array[7]}
	dataset_basepath=${cfg_array[8]}
	query_basepath=${cfg_array[9]}
	experiments_basepath=${cfg_array[10]}

	PWD=$(pwd)

	query_processing_time_file_path="${PWD}/workspace/beva-2-level/experiments/query_processing_time_data_set_${dataset}_size_type_${size_type}_tau_${edit_distance}_index_level_threshold_${index_level_threshold}_max_second_level_size_${max_second_level_size}_alg_${alg}.txt"

	current_query_id=$(head -n 1 ${query_processing_time_file_path})

	if test -z "$current_query_id"
	then
		if [[ "${dataset}" == 0 ]] && [[ "${qry_number_start}" == 0 ]] && [[ "${edit_distance}" == 1 ]] ; then
			echo "<<<<<<<<<< Starting Run Experiments >>>>>>>>>>>"
		else
			echo "Error unknown"
			exit 1 # break
		fi
	else
		if [[ "${current_query_id}" == 999 ]] || [[ "${current_query_id}" == "999" ]] ; then
			recovery_mode="0"
			qry_number_start="0"

			if [[ "${edit_distance}" == 3 ]] ; then
        if [[ "${dataset}" == 4 ]] ; then
          exit 1 # break, program run successfully
        else
          dataset=$(($dataset + 1))
          edit_distance="1"
        fi
      else
        edit_distance=$(($edit_distance + 1))
      fi
		else
			recovery_mode="1"
			qry_number_start=$(($current_query_id + 1))
		fi

		echo -n > ${cfg_path} # Clear file

		echo "edit_distance=${edit_distance}" >> ${cfg_path}
		echo "dataset=${dataset}" >> ${cfg_path}
		echo "queries_size=${queries_size}" >> ${cfg_path}
		echo "recovery_mode=${recovery_mode}" >> ${cfg_path}
		echo "qry_number_start=${qry_number_start}" >> ${cfg_path}
		echo "qry_number_end=${qry_number_end}" >> ${cfg_path}
		echo "size_type=${size_type}" >> ${cfg_path}
		echo "alg=${alg}" >> ${cfg_path}
		echo "dataset_basepath=${dataset_basepath}" >> ${cfg_path}
		echo "query_basepath=${query_basepath}" >> ${cfg_path}
		echo "experiments_basepath=${experiments_basepath}" >> ${cfg_path}

		cat ${cfg_path}

		echo "<<<<<<<<<< Restarting Run Experiments >>>>>>>>>>>"
	fi

  ${free_cache} # Free cache
  ${executable_run_script_bash_path}
	# */15 * * * * /home/berg/workspace/mestrado/beva-2-level/run_job.sh b2lv /home/berg/workspace/mestrado/beva-2-level > /home/berg/workspace/mestrado/beva-2-level/run_job.log 2> /home/berg/workspace/mestrado/beva-2-level/run_job.err
fi
