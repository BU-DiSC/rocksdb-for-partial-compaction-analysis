#!/bin/bash

run_once() {
    if ! [ $# -eq 10 ]; then
        echo 'get the number of parameters:' $#
        echo 'in this function, there will be 10 parameters, which are:'
        echo '1. the number of all inserted bytes'
        echo '2. the path of the rocksdb'
        echo '3. the path of the experiment'
        echo '4. the running method (kRoundRobin, kMinOverlappingRatio, kEnumerateAll)'
        echo '5. the workload path'
        echo '6. write buffer size'
        echo '7. target file size base'
        echo '8. max bytes for level base' 
        echo '9. write buffer data structure'
        echo '10. max bytes for level multiplier'
        exit 1
    fi
    find $2 -mindepth 1 -delete
    ./simple_runner $4 $1 $2 $3 $5 0 0 $6 $7 $8 $9 ${10}
    cp $2/LOG $3/LOG_$4
    rocksdb_size=$(du -sk $2 | awk '{ printf "%dK\n", $1 }')
    echo "$4: $rocksdb_size" >> $3/rocksdb_size.txt
}

run_all_baselines() {
    if ! [ $# -eq 9 ]; then
        echo 'get the number of parameters:' $#
        echo 'in this shell script, there will be 9 parameters, which are:'
        echo '1. the number of all inserted bytes'
        echo '2. the path of the rocksdb'
        echo '3. the path of the experiment workspace'
        echo '4. the workload path'
        echo '5. write buffer size'
        echo '6. target file size base'
        echo '7. max bytes for level base' 
        echo '8. write buffer data structure'
        echo '9. max bytes for level multiplier'
        exit 1
    fi
    find $2 -mindepth 1 -delete
    ./simple_runner kRoundRobin $1 $2 $3 $4 0 0 $5 $6 $7 $8 $9
    cp $2/LOG $3/LOG_RR
    rocksdb_size=$(du -sk $2 | awk '{ printf "%dK\n", $1 }')
    echo "kRoundRobin: $rocksdb_size" >> $3/rocksdb_size.txt

    find $2 -mindepth 1 -delete
    ./simple_runner kMinOverlappingRatio $1 $2 $3 $4 0 0 $5 $6 $7 $8 $9
    cp $2/LOG $3/LOG_MOR
    rocksdb_size=$(du -sk $2 | awk '{ printf "%dK\n", $1 }')
    echo "kMinOverlappingRatio: $rocksdb_size" >> $3/rocksdb_size.txt

    find $2 -mindepth 1 -delete
    ./simple_runner kOldestLargestSeqFirst $1 $2 $3 $4 0 0 $5 $6 $7 $8 $9
    cp $2/LOG $3/LOG_OLSF
    rocksdb_size=$(du -sk $2 | awk '{ printf "%dK\n", $1 }')
    echo "kOldestLargestSeqFirst: $rocksdb_size" >> $3/rocksdb_size.txt

    find $2 -mindepth 1 -delete
    ./simple_runner kOldestSmallestSeqFirst $1 $2 $3 $4 0 0 $5 $6 $7 $8 $9
    cp $2/LOG $3/LOG_OSSF
    rocksdb_size=$(du -sk $2 | awk '{ printf "%dK\n", $1 }')
    echo "kOldestSmallestSeqFirst: $rocksdb_size" >> $3/rocksdb_size.txt
}

run_enumerate() {
    if ! [ $# -eq 12 ]; then
        echo 'get the number of parameters:' $#
        echo 'in this shell script, there will be 12 parameters, which are:'
        echo '1. the number of runs'
        echo '2. the number of all inserted bytes'
        echo '3. the path of the rocksdb'
        echo '4. the path of the experiment workspace'
        echo '5. the workload path'
        echo '6. whether to skip trivial move'
        echo '7. whether to skip extend non-l0 trivial move'
        echo '8. write buffer size'
        echo '9. target file size base'
        echo '10. max bytes for level base' 
        echo '11. write buffer data structure'
        echo '12. max bytes for level multiplier'
        exit 1
    fi
    for i in $(seq 1 $1)
    do
        echo 'run' $i
        find $3 -mindepth 1 -delete
        ./simple_runner kEnumerateAll $2 $3 $4 $5 $6 $7 $8 $9 ${10} ${11} ${12}

        # check whether to stop
        ./check_finish_enumeration $4

        # check whether over exists
        if [ -e $4/over ]; then
            echo 'Stop enumerating'
            rm $4/over
            break
        fi
    done

    echo 'Finish all runs'
}

initialize_workspace() {
    if ! [ $# -eq 1 ]; then
        echo 'get the number of parameters:' $#
        echo 'in this shell script, there will be 1 parameters, which are:'
        echo '1. the path of the experiment workspace'
        exit 1
    fi

    if [ ! -d $1 ]; then
        mkdir $1
    fi
    if [ ! -d $1/history ]; then
        mkdir $1/history
        echo -e 'Number of nodes\n0' > $1/history/picking_history_level0
        echo -e 'Number of nodes\n0' > $1/history/picking_history_level1
    fi

    if [ ! -d $1/minimum.txt ]; then
        echo '18446744073709551615 0' > $1/minimum.txt
    fi

    if [ ! -d $1/log.txt ]; then
        touch $1/log.txt
    fi

    if [ ! -d $1/rocksdb_size.txt ]; then
        touch $1/rocksdb_size.txt
    fi

    if [ ! -d $1/version_info.txt ]; then
        touch $1/version_info.txt
    fi

    if [ ! -d $1/out.txt ]; then
        touch $1/out.txt
    fi
}
