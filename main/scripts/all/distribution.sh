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
    dir_name=compare_distribution/$7/${percentage_insert}_${percentage_update}_${percentage_delete}
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

insert_distribution() {
    num_workloads=10

    # insert distribution
    rocksdb_root_dir=/scratchNVM1/ranw/insert/normal1
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --ID_NMP\ 0.5\ --ID_NDEV\ 0.5 insert_normal1 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --ID_NMP\ 0.5\ --ID_NDEV\ 0.5 insert_normal1 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --ID_NMP\ 0.5\ --ID_NDEV\ 0.5 insert_normal1 &

    rocksdb_root_dir=/scratchNVM1/ranw/insert/normal2
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --ID_NMP\ 0.5\ --ID_NDEV\ 1 insert_normal2 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --ID_NMP\ 0.5\ --ID_NDEV\ 1 insert_normal2 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --ID_NMP\ 0.5\ --ID_NDEV\ 1 insert_normal2 &

    wait $(jobs -p)

    rocksdb_root_dir=/scratchNVM1/ranw/insert/normal3
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --ID_NMP\ 0.5\ --ID_NDEV\ 2 insert_normal3 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --ID_NMP\ 0.5\ --ID_NDEV\ 2 insert_normal3 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --ID_NMP\ 0.5\ --ID_NDEV\ 2 insert_normal3 &

    rocksdb_root_dir=/scratchNVM1/ranw/insert/zipfian1
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --ID_ZALPHA\ 0.7 insert_zipfian1 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --ID_ZALPHA\ 0.7 insert_zipfian1 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --ID_ZALPHA\ 0.7 insert_zipfian1 &

    wait $(jobs -p)

    rocksdb_root_dir=/scratchNVM1/ranw/insert/zipfian2
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --ID_ZALPHA\ 1 insert_zipfian2 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --ID_ZALPHA\ 1 insert_zipfian2 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --ID_ZALPHA\ 1 insert_zipfian2 &

    rocksdb_root_dir=/scratchNVM1/ranw/insert/zipfian3
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --ID_ZALPHA\ 1.3 insert_zipfian3 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --ID_ZALPHA\ 1.3 insert_zipfian3 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --ID_ZALPHA\ 1.3 insert_zipfian3 &
}

update_distribution() {
    num_workloads=10

    # run experiments on normal(0.5, 0.5)
    rocksdb_root_dir=/scratchNVM1/ranw/update/normal1
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 0.5 update_normal1 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 0.5 update_normal1 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 0.5 update_normal1 &

    # run experiments on normal(0.5, 1)
    rocksdb_root_dir=/scratchNVM1/ranw/update/normal2
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 1 update_normal2 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 1 update_normal2 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 1 update_normal2 &

    wait $(jobs -p)

    # run experiments on normal(0.5, 2)
    rocksdb_root_dir=/scratchNVM1/ranw/update/normal3
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 2 update_normal3 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 2 update_normal3 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 1\ --UD_NMP\ 0.5\ --UD_NDEV\ 2 update_normal3 &

    # run experiments on zipfian(0.8)
    rocksdb_root_dir=/scratchNVM1/ranw/update/zipfian1
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 0.7 update_zipfian1 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 0.7 update_zipfian1 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 0.7 update_zipfian1 &

    wait $(jobs -p)

    # # run experiments on zipfian(1)
    rocksdb_root_dir=/scratchNVM1/ranw/update/zipfian2
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 1 update_zipfian2 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 1 update_zipfian2 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 1 update_zipfian2 &

    # # run experiments on zipfian(1.2)
    rocksdb_root_dir=/scratchNVM1/ranw/update/zipfian3
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 1.3 update_zipfian3 &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 1.3 update_zipfian3 &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --UD\ 3\ --UD_ZALPHA\ 1.3 update_zipfian3 &

    wait $(jobs -p)
}

both_distribution() {
    num_workloads=10

    # no distribution
    rocksdb_root_dir=/scratchNVM1/ranw/uniform
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --ID\ 0 uniform &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --ID\ 0 uniform &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --ID\ 0 uniform &

    # both distribution
    rocksdb_root_dir=/scratchNVM1/ranw/both/normal
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --UD\ 1 both_normal &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --UD\ 1 both_normal &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --ID\ 1\ --UD\ 1 both_normal &

    wait $(jobs -p)

    rocksdb_root_dir=/scratchNVM1/ranw/both/beta
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --ID\ 2\ --UD\ 2 both_beta &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --ID\ 2\ --UD\ 2 both_beta &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --ID\ 2\ --UD\ 2 both_beta &
    
    rocksdb_root_dir=/scratchNVM1/ranw/both/zipfian
    run_multiple_times_for_baseline 90 10 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --UD\ 3 both_zipfian &
    run_multiple_times_for_baseline 70 30 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --UD\ 3 both_zipfian &
    run_multiple_times_for_baseline 50 50 0 $num_workloads $rocksdb_root_dir --ID\ 3\ --UD\ 3 both_zipfian &

    wait $(jobs -p)
}

insert_distribution
update_distribution
both_distribution
