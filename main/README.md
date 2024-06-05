# Functionality
Our program can get the minimal write amplification of RocksDB of a given workload.

# How to run
e.g.
./run_all.sh 1000000 0 0 ./tmp 800
here is the explanation to the parameter
1. the number of inserted in the workload
2. the number of updates in the workload
3. the number of deletes in the workload
4. the path to save the experiments result
5. the number of times to run kEnumerate

# What you get
After completing running, you can find there is a folder whose name is the pattern of the workload. In this folder, there are four things. Folder "history" records the picking history. Folder "result" records the write amplification of each run and the minimum write amplification so far. The write amplification of KRoundRobin and KMinOverlappingRatio are recorded in the first two lines in result.txt. Besides, the workload and logs are also included.
