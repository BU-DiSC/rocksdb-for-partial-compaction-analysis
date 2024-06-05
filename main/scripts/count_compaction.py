import os
import regex

def get_compaction_num(file):
    compaction_num = 0
    compaction_num_in_L0 = 0
    # compaction_num_in_L1 = 0
    flush_num = 0
    file_nums_to_L1 = []
    # file_nums_to_L2 = []
    with open(file, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if "[/compaction/compaction_job.cc:1748]" in line:
                compaction_num += 1
                if "to L1" in line:
                    compaction_num_in_L0 += 1
                    # get the number before "@0" in "[default] [JOB 6] Compacted 4@0 files to L1"
                    file_nums_to_L1.append(int(regex.findall(r'\d+(?=@0)', line)[0]))
                # if "to L2" in line:
                #     compaction_num_in_L1 += 1
                #     # get the number before "@0" in "[default] [JOB 6] Compacted 4@0 files to L1"
                #     file_nums_to_L2.append(int(regex.findall(r'\d+(?=@1)', line)[0]))
            elif "/db_impl/db_impl_compaction_flush.cc:3226" in line:
                flush_num += 1

    # print("compaction num: ", compaction_num)
    # print("compaction in L0: ", compaction_num_in_L0)
    avg_file_num_to_L1 = sum(file_nums_to_L1) / len(file_nums_to_L1)
    return compaction_num, compaction_num_in_L0, flush_num, avg_file_num_to_L1

def get_avg_compaction_num(data_root):
    log_file_names = ['LOG_RR', 'LOG_MOR', 'LOG_OLSF', 'LOG_OSSF']
    compaction_nums = {k: [] for k in log_file_names}
    compaction_nums_in_L0 = {k: [] for k in log_file_names}
    flush_nums = {k: [] for k in log_file_names}
    avg_file_nums_to_L1 = {k: [] for k in log_file_names}
    # traverse all sub directories
    for dir in os.listdir(data_root):
        if os.path.isdir(data_root + '/' + dir):
            for log_file_name in log_file_names:
                compaction_num, compaction_num_in_L0, flush_num, avg_file_num_to_L1 = get_compaction_num(data_root + '/' + dir + '/' + log_file_name)
                compaction_nums[log_file_name].append(compaction_num)
                compaction_nums_in_L0[log_file_name].append(compaction_num_in_L0)
                flush_nums[log_file_name].append(flush_num)
                avg_file_nums_to_L1[log_file_name].append(avg_file_num_to_L1)
    return compaction_nums, compaction_nums_in_L0, flush_nums, avg_file_nums_to_L1

# data_root = 'workspace/edbt/compare_proportion'
# data_root = 'workspace/edbt/compare_distribution/update_zipfian3'
data_root = 'workspace/edbt/compare_devices'
# sub_dirs = ['100_0_0', '90_10_0', '80_20_0', '70_30_0', '60_40_0', '50_50_0', '40_60_0', '30_70_0', '20_80_0', '10_90_0']
# sub_dirs = ['90_10_0', '70_30_0', '50_50_0']
sub_dirs = ['100_0/nvme1', '90_10/nvme1', '80_20/nvme1', '70_30/nvme1', '60_40/nvme1', '50_50/nvme1']
# sub_dirs = ['100_0/ssd', '90_10/ssd', '80_20/ssd', '70_30/ssd', '60_40/ssd', '50_50/ssd']
log_file_names = ['LOG_RR', 'LOG_MOR', 'LOG_OLSF', 'LOG_OSSF']

for dir in sub_dirs:
    compaction_nums, compaction_nums_in_L0, flush_nums, avg_file_nums_to_L1 = get_avg_compaction_num(data_root + '/' + dir)
    print("dir: ", dir)
    for log_file_name in log_file_names:
        avg_compaction_num = compaction_nums[log_file_name]
        avg_compaction_num_in_L0 = compaction_nums_in_L0[log_file_name]
        avg_flush_num = flush_nums[log_file_name]
        avg_file_num_to_L1 = avg_file_nums_to_L1[log_file_name]
        print(log_file_name)
        print("Average compaction num: ", sum(avg_compaction_num) / len(avg_compaction_num))
        print("Average compaction num in L0: ", sum(avg_compaction_num_in_L0) / len(avg_compaction_num_in_L0))
        print("Percentage of compaction in L0: ", sum(avg_compaction_num_in_L0) / sum(avg_compaction_num) * 100)
        print("Average flush num: ", sum(avg_flush_num) / len(avg_flush_num))
        print("Average file num to L1: ", sum(avg_file_num_to_L1) / len(avg_file_num_to_L1))

