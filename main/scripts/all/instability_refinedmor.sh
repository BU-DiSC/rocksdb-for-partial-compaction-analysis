source ./scripts/run_workload.sh

run_multiple_times_for_baseline() {
    if ! [ $# -eq 10 ]; then
        echo 'get the number of parameters:' $#
        echo 'in this shell script, there will be 10 parameters, which are:'
        echo '1. percentage_insert'
        echo '2. percentage_update'
        echo '3. percentage_delete'
        echo '4. the number of workloads'
        echo '5. rocksdb_root_dir'
        echo '6. update distribution'
        echo '7. experiment name'
        echo '8. buffer size'
        echo '9. operation number'
        echo '10. entry size'
        exit 1
    fi

    write_buffer_size=$8
    target_file_size_base=$8
    max_bytes_for_level_base=$((4 * 8 * 1024 * 1024))

    percentage_insert=$1
    percentage_update=$2
    percentage_delete=$3
    n_workloads=$4
    num_operation=$9
    entry_size=${10}

    num_insert=$((num_operation * percentage_insert / 100))
    num_update=$((num_operation * percentage_update / 100))
    num_delete=$((num_operation * percentage_delete / 100))
    workload_size=$(((num_insert + num_update) * entry_size))
    dir_name=instability_mor/${num_operation}_${entry_size}/${percentage_insert}_${percentage_update}_${percentage_delete}
    workload_dir=workloads/edbt/$dir_name
    workspace_dir=workspace/edbt/$dir_name
    mkdir -p $workload_dir
    mkdir -p $workspace_dir
    
    rocksdb_dir=$5/${percentage_insert}_${percentage_update}_${percentage_delete}
    mkdir -p $rocksdb_dir

    for i in $(seq 1 $n_workloads)
    do  
        ./load_gen --output_path $workload_dir/$i.txt -I $num_insert -U $num_update -D $num_delete -E $entry_size -K 8 $6
        initialize_workspace $workspace_dir/run$i
        run_once $workload_size $rocksdb_dir $workspace_dir/run$i kRefinedMOR $workload_dir/$i.txt $write_buffer_size $target_file_size_base $max_bytes_for_level_base Vector 4
        rm $workload_dir/$i.txt
    done

    rm -rf $rocksdb_dir
}

num_workloads=50
rocksdb_root_dir=/scratchNVM1/ranw/instability_mor
experiment_name=instability_mor
buffer_size=$((8 * 1024 * 1024))
num_operation=8000000
entry_size=8
run_multiple_times_for_baseline 100 0 0 $num_workloads $rocksdb_root_dir --UD\ 0 $experiment_name $buffer_size $num_operation $entry_size

num_workloads=50
rocksdb_root_dir=/scratchNVM1/ranw/instability_mor
experiment_name=instability_mor
buffer_size=$((8 * 1024 * 1024))
num_operation=2000000
entry_size=64
run_multiple_times_for_baseline 100 0 0 $num_workloads $rocksdb_root_dir --UD\ 0 $experiment_name $buffer_size $num_operation $entry_size

