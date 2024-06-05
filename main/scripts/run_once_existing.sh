#!/bin/bash

source ./scripts/run_workload.sh

if ! [ $# -eq 10 ]; then
    echo 'get the number of parameters:' $#
    echo 'in this shell script, there will be 10 parameters, which are:'
    echo '1. the path of rocksdb'
    echo '2. the path of the experiment workspace'
    echo '3. running method (kRoundRobin, kMinOverlappingRatio, kEnumerateAll, kManual, kTwoStepsSearch, kRefinedMOR)'
    echo '4. the number of all inserted bytes'
    echo '5. the workload path'
    echo '6. write buffer size'
    echo '7. target file size base'
    echo '8. max bytes for level base' 
    echo '9. write_buffer_data_structure'
    echo '10. max_bytes_for_level_base_multiplier'
    exit 1
fi

# initialize the workspace
initialize_workspace $2

# run once
run_once $4 $1 $2 $3 $5 $6 $7 $8 $9 ${10}
