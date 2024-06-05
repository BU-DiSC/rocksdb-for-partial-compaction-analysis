source ./scripts/run_workload.sh

run_multiple_times_for_baseline() {
    if ! [ $# -eq 7 ]; then
        echo 'get the number of parameters:' $#
        echo 'in this shell script, there will be 7 parameters, which are:'
        echo '1. percentage_insert'
        echo '2. percentage_update'
        echo '3. percentage_delete'
        echo '4. the number of workloads'
        echo '5. rocksdb_root_dir'
        echo '6. update distribution'
        echo '7. experiment name'
        exit 1
    fi

    write_buffer_size=$((64 * 1024 * 1024))
    target_file_size_base=$((64 * 1024 * 1024))
    max_bytes_for_level_base=$((4 * 64 * 1024 * 1024))

    percentage_insert=$1
    percentage_update=$2
    percentage_delete=$3
    n_workloads=$4
    entry_size=1024
    num_operation=$((5 * 1024 * 1024))

    num_insert=$((num_operation * percentage_insert / 100))
    num_update=$((num_operation * percentage_update / 100))
    num_delete=$((num_operation * percentage_delete / 100))
    workload_size=$(((num_insert + num_update) * entry_size))
    dir_name=compare_distribution/update_distribution_influence/$7/${percentage_insert}_${percentage_update}_${percentage_delete}
    workload_dir=workloads/edbt/$dir_name
    workspace_dir=workspace/edbt/$dir_name
    mkdir -p $workload_dir
    mkdir -p $workspace_dir
    
    rocksdb_dir=$5/${percentage_insert}_${percentage_update}_${percentage_delete}
    mkdir -p $rocksdb_dir

    for i in $(seq 1 $n_workloads)
    do  
        ./load_gen --output_path $workload_dir/${i}.txt -I $num_insert -U $num_update -D $num_delete -E $entry_size -K 8 $6
        initialize_workspace $workspace_dir/run$i
        run_all_baselines $workload_size $rocksdb_dir $workspace_dir/run$i $workload_dir/${i}.txt $write_buffer_size $target_file_size_base $max_bytes_for_level_base Vector 4
        rm $workload_dir/${i}.txt
    done

    rm -rf $rocksdb_dir
}

update_distribution() {
    num_workloads=10

    # normal distribution
    rocksdb_root_dir=/scratchNVM1/ranw/update/normal1
    experiment_name=update_distribution_influence/normal1
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 0.001 $experiment_name &

    rocksdb_root_dir=/scratchNVM1/ranw/update/normal2
    experiment_name=update_distribution_influence/normal2
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 0.01 $experiment_name &

    rocksdb_root_dir=/scratchNVM1/ranw/update/normal3
    experiment_name=update_distribution_influence/normal3
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 0.1 $experiment_name &

    rocksdb_root_dir=/scratchNVM1/ranw/update/normal4
    experiment_name=update_distribution_influence/normal4
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 1 $experiment_name &

    rocksdb_root_dir=/scratchNVM1/ranw/update/normal5
    experiment_name=update_distribution_influence/normal5
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 10 $experiment_name &

    rocksdb_root_dir=/scratchNVM1/ranw/update/normal6
    experiment_name=update_distribution_influence/normal6
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 100 $experiment_name &

    wait $(jobs -p)

    # zipfian distribution
    rocksdb_root_dir=/scratchNVM1/ranw/update/zipfian1
    experiment_name=update_distribution_influence/zipfian1
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 0.1 $experiment_name &

    rocksdb_root_dir=/scratchNVM1/ranw/update/zipfian2
    experiment_name=update_distribution_influence/zipfian2
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 0.4 $experiment_name &

    rocksdb_root_dir=/scratchNVM1/ranw/update/zipfian3
    experiment_name=update_distribution_influence/zipfian3
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 0.7 $experiment_name &

    rocksdb_root_dir=/scratchNVM1/ranw/update/zipfian4
    experiment_name=update_distribution_influence/zipfian4
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 1.0 $experiment_name &

    rocksdb_root_dir=/scratchNVM1/ranw/update/zipfian5
    experiment_name=update_distribution_influence/zipfian5
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 1.3 $experiment_name &

    rocksdb_root_dir=/scratchNVM1/ranw/update/zipfian6
    experiment_name=update_distribution_influence/zipfian6
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 1.6 $experiment_name &

    wait $(jobs -p)
}

update_distribution
