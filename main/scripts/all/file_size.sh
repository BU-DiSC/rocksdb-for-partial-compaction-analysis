source ./scripts/run_workload_backup.sh

run_multiple_times_for_baseline() {
    if ! [ $# -eq 13 ]; then
        echo 'get the number of parameters:' $#
        echo 'in this shell script, there will be 13 parameters, which are:'
        echo '1. percentage_insert'
        echo '2. percentage_update'
        echo '3. the number of workloads'
        echo '4. rocksdb_root_dir'
        echo '5. num_operation'
        echo '6. entry_size'
        echo '7. experiment name'
        echo '8. write buffer size'
        echo '9. target file size base'
        echo '10. max bytes for level base'
        echo '11. max bytes for level multiplier'
        echo '12. generate workload or not'
        echo '13. delete workload or not'
        exit 1
    fi

    write_buffer_size=$8
    target_file_size_base=$9
    max_bytes_for_level_base=${10}
    max_bytes_for_level_multiplier=${11}
    generate_workload=${12}
    delete_workload=${13}

    percentage_insert=$1
    percentage_update=$2
    n_workloads=$3
    num_operation=$5
    entry_size=$6
    workload_size=$((num_operation * entry_size))

    num_insert=$((num_operation * percentage_insert / 100))
    num_update=$((num_operation * percentage_update / 100))
    dir_name=file_size/$7/$max_bytes_for_level_multiplier/${percentage_insert}_${percentage_update}
    workload_dir=workloads/edbt/file_size/$7/${percentage_insert}_${percentage_update}
    workspace_dir=workspace/edbt/$dir_name
    mkdir -p $workload_dir
    mkdir -p $workspace_dir
    
    rocksdb_dir=$4/${percentage_insert}_${percentage_update}
    mkdir -p $rocksdb_dir

    for i in $(seq 1 $n_workloads)
    do  
        if [ $generate_workload -eq 1 ]; then
            ./load_gen --output_path $workload_dir/${i}.txt -I $num_insert -U $num_update -D 0 -E $entry_size -K 8
        fi
        initialize_workspace $workspace_dir/run$i
        run_all_baselines $workload_size $rocksdb_dir $workspace_dir/run$i $workload_dir/${i}.txt $write_buffer_size $target_file_size_base $max_bytes_for_level_base Vector $max_bytes_for_level_multiplier
        if [ $delete_workload -eq 1 ]; then
            rm $workload_dir/${i}.txt
        fi
    done

    rm -rf $rocksdb_dir
}

file_size() {
    num_workloads=10

    num_operation=$((5 * 1024 * 1024))
    entry_size=1024
    buffer_size=$((64 * 1024 * 1024))

    # 100% insert

    rocksdb_root_dir=/scratchNVM1/ranw/file_size/16M
    experiment_name=16M
    file_size_base=$((16 * 1024 * 1024))
    max_bytes_for_level_base=$((256 * 1024 * 1024))
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 4 1 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 6 1 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 8 0 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 10 0 1

    rocksdb_root_dir=/scratchNVM1/ranw/file_size/32M
    experiment_name=32M
    file_size_base=$((32 * 1024 * 1024))
    max_bytes_for_level_base=$((256 * 1024 * 1024))
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 4 1 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 6 1 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 8 0 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 10 0 1

    rocksdb_root_dir=/scratchNVM1/ranw/file_size/64M
    experiment_name=64M
    file_size_base=$((64 * 1024 * 1024))
    max_bytes_for_level_base=$((256 * 1024 * 1024))
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 4 1 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 6 1 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 8 0 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 10 0 1

    rocksdb_root_dir=/scratchNVM1/ranw/file_size/128M
    experiment_name=128M
    file_size_base=$((128 * 1024 * 1024))
    max_bytes_for_level_base=$((256 * 1024 * 1024))
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 4 1 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 6 1 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 8 0 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 10 0 1

    rocksdb_root_dir=/scratchNVM1/ranw/file_size/5G
    experiment_name=5G
    file_size_base=$((5 * 1024 * 1024 * 1024))
    max_bytes_for_level_base=$((256 * 1024 * 1024))
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 4 1 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 6 1 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 8 0 0
    run_multiple_times_for_baseline 100 0 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 10 0 1

    # 50% insert, 50% update

    rocksdb_root_dir=/scratchNVM1/ranw/file_size/16M
    experiment_name=16M
    file_size_base=$((16 * 1024 * 1024))
    max_bytes_for_level_base=$((256 * 1024 * 1024))16
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 4 1 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 6 1 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 8 0 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 10 0 1

    rocksdb_root_dir=/scratchNVM1/ranw/file_size/32M
    experiment_name=32M
    file_size_base=$((32 * 1024 * 1024))
    max_bytes_for_level_base=$((256 * 1024 * 1024))
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 4 1 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 6 1 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 8 0 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 10 0 1

    rocksdb_root_dir=/scratchNVM1/ranw/file_size/64M
    experiment_name=64M
    file_size_base=$((64 * 1024 * 1024))
    max_bytes_for_level_base=$((256 * 1024 * 1024))
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 4 1 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 6 1 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 8 0 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 10 0 1

    rocksdb_root_dir=/scratchNVM1/ranw/file_size/128M
    experiment_name=128M
    file_size_base=$((128 * 1024 * 1024))
    max_bytes_for_level_base=$((256 * 1024 * 1024))
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 4 1 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 6 1 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 8 0 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 10 0 1

    rocksdb_root_dir=/scratchNVM1/ranw/file_size/5G
    experiment_name=5G
    file_size_base=$((5 * 1024 * 1024 * 1024))
    max_bytes_for_level_base=$((256 * 1024 * 1024))
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 4 1 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 6 1 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 8 0 0
    run_multiple_times_for_baseline 50 50 $num_workloads $rocksdb_root_dir $num_operation $entry_size $experiment_name $buffer_size $file_size_base $max_bytes_for_level_base 10 0 1
}

file_size
