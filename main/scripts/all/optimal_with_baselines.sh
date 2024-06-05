source ./scripts/run_workload.sh

run_multiple_times_for_a_type() {
    if ! [ $# -eq 4 ]; then
        echo 'get the number of parameters:' $#
        echo 'in this shell script, there will be 4 parameters, which are:'
        echo '1. percentage_insert'
        echo '2. percentage_update'
        echo '3. percentage_delete'
        echo '4. the number of workloads'
        exit 1
    fi

    write_buffer_size=$((8 * 1024 * 1024))
    target_file_size_base=$((8 * 1024 * 1024))
    max_bytes_for_level_base=$((4 * 8 * 1024 * 1024))

    percentage_insert=$1
    percentage_update=$2
    percentage_delete=$3
    n_workloads=$4
    
    entry_size=64
    num_operation=$((3 * 1024 * 1024))
    num_insert=$((num_operation * percentage_insert / 100))
    num_update=$((num_operation * percentage_update / 100))
    num_delete=$((num_operation * percentage_delete / 100))
    workload_size=$(((num_insert + num_update) * entry_size))
    experiment_root_name=edbt/compare_optimal_with_baselines
    experiment_name=${percentage_insert}_${percentage_update}_${percentage_delete}
    dir_name=$experiment_root_name/$experiment_name
    workload_dir=workloads/$dir_name
    workspace_dir=workspace/$dir_name
    mkdir -p $workload_dir
    mkdir -p $workspace_dir
    
    rocksdb_dir=/mnt/ramd/ranw/$dir_name
    mkdir -p $rocksdb_dir

    enumeration_runs=30000

    for i in $(seq 1 $n_workloads)
    do  
        ./load_gen --output_path $workload_dir/${i}.txt -I $num_insert -U $num_update -D $num_delete -E $entry_size -K 8
        ./scripts/run_for_a_type.sh $enumeration_runs $rocksdb_dir/rocksdb$i/ $workspace_dir/run$i $workload_dir/${i}.txt $workload_size 1 0 $write_buffer_size $target_file_size_base $max_bytes_for_level_base Vector 4
    done

    rm -rf $rocksdb_dir
}

num_workloads=10

run_multiple_times_for_a_type 100 0 0 $num_workloads &
run_multiple_times_for_a_type 90 10 0 $num_workloads &
run_multiple_times_for_a_type 80 20 0 $num_workloads &
run_multiple_times_for_a_type 70 30 0 $num_workloads &
run_multiple_times_for_a_type 60 40 0 $num_workloads &
run_multiple_times_for_a_type 50 50 0 $num_workloads

