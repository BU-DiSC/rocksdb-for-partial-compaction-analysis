source ./scripts/run_workload.sh

run_multiple_times_for_baseline() {
    if ! [ $# -eq 3 ]; then
        echo 'get the number of parameters:' $#
        echo 'in this shell script, there will be 3 parameters, which are:'
        echo '1. percentage_insert'
        echo '2. percentage_update'
        echo '3. the number of workloads'
        exit 1
    fi

    write_buffer_size=$((64 * 1024 * 1024))
    target_file_size_base=$((64 * 1024 * 1024))
    max_bytes_for_level_base=$((4 * 64 * 1024 * 1024))

    percentage_insert=$1
    percentage_update=$2
    n_workloads=$3
    workload_size=$((5 * 1024 * 1024 * 1024))
    entry_size=1024
    num_operation=$((workload_size / entry_size))

    num_insert=$((num_operation * percentage_insert / 100))
    num_update=$((num_operation * percentage_update / 100))
    dir_name=compare_devices/${percentage_insert}_${percentage_update}
    workload_dir=workloads/edbt/$dir_name
    workspace_dir_nvme1=workspace/edbt/$dir_name/nvme1
    workspace_dir_ssd=workspace/edbt/$dir_name/ssd
    mkdir -p $workload_dir
    mkdir -p $workspace_dir_nvme1
    mkdir -p $workspace_dir_ssd
    
    rocksdb_dir_nvme1=/scratchNVM1/ranw/nvme1/${percentage_insert}_${percentage_update}
    rocksdb_dir_ssd=/scratchSSD/ranw/ssd/${percentage_insert}_${percentage_update}

    mkdir -p $rocksdb_dir_nvme1
    mkdir -p $rocksdb_dir_ssd

    for i in $(seq 1 $n_workloads)
    do  
        ./load_gen --output_path $workload_dir/${i}.txt -I $num_insert -U $num_update -D 0 -E $entry_size -K 8

        initialize_workspace $workspace_dir_nvme1/run$i
        run_all_baselines $workload_size $rocksdb_dir_nvme1 $workspace_dir_nvme1/run$i $workload_dir/${i}.txt $write_buffer_size $target_file_size_base $max_bytes_for_level_base Vector 4

        initialize_workspace $workspace_dir_ssd/run$i
        run_all_baselines $workload_size $rocksdb_dir_ssd $workspace_dir_ssd/run$i $workload_dir/${i}.txt $write_buffer_size $target_file_size_base $max_bytes_for_level_base Vector 4
        
        rm $workload_dir/${i}.txt
    done

    rm -rf $rocksdb_dir_nvme1
    rm -rf $rocksdb_dir_ssd
}

num_workloads=10

# run experiments on SSD & NVMe
run_multiple_times_for_baseline 100 0 $num_workloads &
run_multiple_times_for_baseline 90 10 $num_workloads &
run_multiple_times_for_baseline 80 20 $num_workloads &

wait $(jobs -p)

run_multiple_times_for_baseline 70 30 $num_workloads &
run_multiple_times_for_baseline 60 40 $num_workloads &
run_multiple_times_for_baseline 50 50 $num_workloads &

wait $(jobs -p)
