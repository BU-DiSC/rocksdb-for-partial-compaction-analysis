source ./scripts/run_workload.sh

run_multiple_times_for_a_type() {
    if ! [ $# -eq 7 ]; then
        echo 'get the number of parameters:' $#
        echo 'in this shell script, there will be 7 parameters, which are:'
        echo '1. percentage_insert'
        echo '2. percentage_update'
        echo '3. the number of workloads'
        echo '4. entry size'
        echo '5. number of operations'
        echo '6. type'
        echo '7. param 1'
        echo '8. param 2'
        exit 1
    fi

    write_buffer_size=$((8 * 1024 * 1024))
    target_file_size_base=$((8 * 1024 * 1024))
    max_bytes_for_level_base=$((4 * 8 * 1024 * 1024))

    percentage_insert=$1
    percentage_update=$2
    n_workloads=$3
    
    entry_size=$4
    num_operation=$5
    workload_size=$((num_operation * entry_size))

    type=$6
    param_1=$7
    param_2=$8

    num_insert=$((num_operation * percentage_insert / 100))
    num_update=$((num_operation * percentage_update / 100))
    experiment_root_name=edbt/compare_optimal_policies
    experiment_name=${percentage_insert}_${percentage_update}
    dir_name=$experiment_root_name/$experiment_name

    workload_dir=workloads/$dir_name/$type
    mkdir -p $workload_dir

    workspace_dir=workspace/$dir_name/$type
    mkdir -p $workspace_dir
    
    rocksdb_dir=/mnt/ramd/ranw/$dir_name/$type
    mkdir -p $rocksdb_dir

    enumeration_runs=10000

    for i in $(seq 1 $n_workloads)
    do  
        ./load_gen --output_path $workload_dir/${i}.txt -I $num_insert -U $num_update -D 0 -E $entry_size -K 8
        ./scripts/run_for_a_type.sh $enumeration_runs $rocksdb_dir/rocksdb$i/ $workspace_dir/run$i $workload_dir/${i}.txt $workload_size $param_1 $param_2 $write_buffer_size $target_file_size_base $max_bytes_for_level_base Vector 4 &
    done

    wait $(jobs -p) 

    rm -rf $rocksdb_dir
}

num_workloads=10
entry_size=64
num_operation=2000000

run_multiple_times_for_a_type 100 0 $num_workloads $entry_size $num_operation skip 1 0 &
run_multiple_times_for_a_type 80 20 $num_workloads $entry_size $num_operation skip 1 0 &
run_multiple_times_for_a_type 60 40 $num_workloads $entry_size $num_operation skip 1 0 &

wait $(jobs -p)

run_multiple_times_for_a_type 100 0 $num_workloads $entry_size $num_operation non_skip 0 0 &
run_multiple_times_for_a_type 80 20 $num_workloads $entry_size $num_operation non_skip 0 0 &
run_multiple_times_for_a_type 60 40 $num_workloads $entry_size $num_operation non_skip 0 0 &

wait $(jobs -p)

run_multiple_times_for_a_type 100 0 $num_workloads $entry_size $num_operation optimal 1 1 &
run_multiple_times_for_a_type 80 20 $num_workloads $entry_size $num_operation optimal 1 1 &
run_multiple_times_for_a_type 60 40 $num_workloads $entry_size $num_operation optimal 1 1 &

wait $(jobs -p)
