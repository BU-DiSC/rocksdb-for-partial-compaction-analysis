import os
import numpy as np

def read_minimum(directory):
    minimum_file = open(os.path.join(directory, 'minimum.txt'), 'r')
    # read the first line of minimum file and get the first number
    minimum = int(minimum_file.readline().split()[0])
    minimum_file.close()
    return minimum

def read_result(directory):
    result_file = open(os.path.join(directory, 'result.txt'), 'r')
    result_file_data = result_file.read().split("\n")[1:]
    result = {}
    for line in result_file_data:
        if len(line) == 0:
            continue
        res = line.split("\t")
        result[res[0]] = [int(res[1]), float(res[2]), int(res[3])]
    result_file.close()
    return result

def analyze_a_type(directory):
    total_bytes = 2000000 * 64
    result = []
    minimums = []
    # get the subdirectories in one layer deep
    subdirectories = [os.path.join(directory, d) for d in os.listdir(directory) if os.path.isdir(os.path.join(directory, d))]
    for dir in subdirectories:
        print(dir)
        result.append(read_result(dir))
        minimums.append(read_minimum(dir))
    # compute average, maximum, minimum, median, mean, and standard deviation
    accumulative = {}
    for res in result:
        for k, v in res.items():
            if k not in accumulative:
                accumulative[k] = []
            accumulative[k].append(v[0])
    accumulative['minimum'] = minimums
    accumulative_wa = {}
    for res in result:
        for k, v in res.items():
            if k not in accumulative_wa:
                accumulative_wa[k] = []
            accumulative_wa[k].append(v[1])
    accumulative_duration = {}
    for res in result:
        for k, v in res.items():
            if k not in accumulative_duration:
                accumulative_duration[k] = []
            accumulative_duration[k].append(v[2])
    statistics = {}
    statistics['minimum'] = {}
    v = accumulative['minimum']
    statistics['minimum']['mean'] = np.mean(v)
    statistics['minimum']['median'] = np.median(v)
    statistics['minimum']['std'] = np.std(v)
    statistics['minimum']['min'] = np.min(v)
    statistics['minimum']['max'] = np.max(v)
    statistics['minimum']['25th'] = np.percentile(v, 25)
    statistics['minimum']['75th'] = np.percentile(v, 75)
    statistics['minimum']['duration'] = -1
    statistics['minimum']['wa'] = statistics['minimum']['mean'] / total_bytes
    statistics['minimum']['distance'] = -1

    for k, v in accumulative.items():
        if k == 'minimum':
            continue
        if k not in statistics:
            statistics[k] = {}
        mean_array = np.sort(np.array(v))[1:-1]
        statistics[k]['mean'] = np.mean(mean_array)
        statistics[k]['mean'] = np.mean(v)
        statistics[k]['median'] = np.median(v)
        statistics[k]['std'] = np.std(v)
        statistics[k]['min'] = np.min(v)
        statistics[k]['max'] = np.max(v)
        statistics[k]['25th'] = np.percentile(v, 25)
        statistics[k]['75th'] = np.percentile(v, 75)
        statistics[k]['duration'] = np.mean(accumulative_duration[k])
        statistics[k]['wa'] = np.mean(accumulative_wa[k])
        # statistics[k]['distance'] = (statistics[k]['mean'] - statistics['minimum']['mean']) / statistics[k]['mean'] * 100
    
    # output the result
    output_file = open(os.path.join(directory, 'result.txt'), 'w')
    # output_file.write("Strategy\tMean\tMedian\tStd\tMin\tMax\t25th\t75th\tduration(us)\tavg WA\tDistance(%)\n")
    # for k, v in statistics.items():
    #     output_file.write("%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\n" % (k, v['mean'], v['median'], v['std'], v['min'], v['max'], v['25th'], v['75th'], v['duration'], v['wa'], v['distance']))
    output_file.write("Strategy\tMean\tMedian\tStd\tMin\tMax\t25th\t75th\tduration(us)\tavg WA\n")
    for k, v in statistics.items():
        output_file.write("%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%f\n" % (k, v['mean'], v['median'], v['std'], v['min'], v['max'], v['25th'], v['75th'], v['duration'], v['wa']))

def analyze_minimum(directory):
    total_bytes = 2000000 * 64
    minimums = []
    # get the subdirectories in one layer deep
    subdirectories = [os.path.join(directory, d) for d in os.listdir(directory) if os.path.isdir(os.path.join(directory, d))]
    for dir in subdirectories:
        minimums.append(read_minimum(dir))
    # compute average, maximum, minimum, median, mean, and standard deviation
    accumulative = {}
    accumulative['minimum'] = minimums
    accumulative['minimum'] = minimums
    statistics = {}
    statistics['minimum'] = {}
    v = accumulative['minimum']
    statistics['minimum']['mean'] = np.mean(v)
    statistics['minimum']['median'] = np.median(v)
    statistics['minimum']['std'] = np.std(v)
    statistics['minimum']['min'] = np.min(v)
    statistics['minimum']['max'] = np.max(v)
    statistics['minimum']['25th'] = np.percentile(v, 25)
    statistics['minimum']['75th'] = np.percentile(v, 75)
    statistics['minimum']['duration'] = -1
    statistics['minimum']['wa'] = statistics['minimum']['mean'] / total_bytes
    statistics['minimum']['distance'] = -1

    # output the result
    output_file = open(os.path.join(directory, 'result.txt'), 'w')
    # output_file.write("Strategy\tMean\tMedian\tStd\tMin\tMax\t25th\t75th\tduration(us)\tavg WA\tDistance(%)\n")
    # for k, v in statistics.items():
    #     output_file.write("%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%f\t%f\n" % (k, v['mean'], v['median'], v['std'], v['min'], v['max'], v['25th'], v['75th'], v['duration'], v['wa'], v['distance']))
    output_file.write("Strategy\tMean\tMedian\tStd\tMin\tMax\t25th\t75th\tduration(us)\tavg WA\n")
    for k, v in statistics.items():
        output_file.write("%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%f\n" % (k, v['mean'], v['median'], v['std'], v['min'], v['max'], v['25th'], v['75th'], v['duration'], v['wa']))


def arrange(directory):
    # output file
    output_file = open(os.path.join(directory, 'result.txt'), 'w')
    result = {}
    for dir in os.listdir(directory):
        if not os.path.isdir(os.path.join(directory, dir)):
            continue
        dir_name = os.path.join(directory, dir)
        print(dir_name)
        file = open(os.path.join(dir_name, 'result.txt'), 'r')
        data = file.read()
        # output_file.write(dir + '\n')
        # output_file.write(data + '\n')
        result[dir] = data
        file.close()
    # sort by key in descending order
    result = sorted(result.items(), key=lambda x: x[0], reverse=True)
    # output the result
    for res in result:
        output_file.write(res[0] + '\n')
        output_file.write(res[1] + '\n')
    output_file.close()

def collect_depth_1():
    ''' root path '''
    # root_path = 'workspace/edbt/compare_distribution/proportion'
    # root_path = 'workspace/edbt/compare_devices'
    # root_path = 'workspace/edbt/file_size'
    root_path = 'workspace/edbt/compare_workload_size'

    ''' sub dir '''
    # names = ['50_50', '60_40', '70_30', '80_20', '90_10', '100_0']
    # names = ['50_50_0', '60_40_0', '70_30_0', '80_20_0', '90_10_0', '100_0_0']
    # names = ['10_90_0', '20_80_0', '30_70_0', '40_60_0', '50_50_0', '60_40_0', '70_30_0', '80_20_0', '90_10_0', '100_0_0']
    # names = ['80_10_10', '80_12_8', '80_14_6', '80_16_4', '80_18_2']
    # names = ['100_0_5', '100_0_10', '100_0_15', '100_0_20', '100_0_25', '100_0_30', '100_0_35', '100_0_40', '100_0_45', '100_0_50']
    # names = ['50_50_5', '50_50_10', '50_50_15', '50_50_20', '50_50_25', '50_50_30']
    # names = ['50_50_0', '75_25_0', '100_0_0']
    names = ['50_50', '75_25', '100_0']

    # walk through
    for name in names:
        directory = root_path + '/' + name
        analyze_a_type(directory)

    # for dir in os.listdir(root_path):
    #     if not os.path.isdir(os.path.join(root_path, dir)):
    #         continue
    #     sub_path = os.path.join(root_path, dir)
    #     analyze_a_type(sub_path)

def collect_depth_2():
    ''' root path '''
    # root_path = 'workspace/edbt/compare_optimal_policies'
    # root_path = 'workspace/edbt/compare_optimal_with_baselines'
    # root_path = 'workspace/edbt/compare_distribution'
    # root_path = 'workspace/edbt/compare_workload_size'
    # root_path = 'workspace/edbt/compare_devices'
    root_path = 'workspace/edbt/file_size'

    ''' sub dir 1 '''
    # names_1 = ['normal1', 'normal2', 'normal3', 'zipfian1', 'zipfian2', 'zipfian3']
    # names_1 = ['normal1', 'normal2', 'normal3', 'normal4', 'normal5', 'normal6']
    # names_1 = ['50_50', '60_40', '70_30', '80_20', '90_10', '100_0']
    # names_1 = ['80_10_10', '80_12_8', '80_14_6', '80_16_4', '80_18_2']
    # names_1 = ['size_5_2048', 'size_5_4096', 'size_5_8192', 'size_10_1024', 'size_20_1024', 'size_40_1024']
    # names_1 = ['40_1024_size', '80_512_size', '160_256_size']
    # names_1 = ['80_512_size', '40_1024_size', '20_2048_size']
    names_1 = ['40_1024_size', '20_512_size', '40_512_size']
    # names_1 = ['update_distribution_influence']
    # names_1 = ['size_5_2048', 'size_5_4096', 'size_5_8192', 'size_10_1024', 'size_20_1024', 'size_40_1024']
    # names_1 = ['1000000_64', '2500000_64']
    # names_1 = ['100_0', '80_20', '60_40']
    # names_1 = ['16M', '32M', '64M', '128M']

    ''' sub dir 2 '''
    # names_2 = ['90_10_0', '70_30_0', '50_50_0']
    # names_2 = ['nvme1', 'ssd']
    # names_2 = ['50_50', '75_25', '100_0']
    # names_2 = ['50_50', '30_70', '10_90']
    # names_2 = ['50_50_0']
    names_2 = ['100_0', '75_25', '50_50']
    # names_2 = ['100_0_0', '75_25_0', '50_50_0']
    # names_2 = ['50_50']
    # names_2 = ['skip', 'non_skip', 'optimal']
    # names_2 = ['50_50', '100_0']

    # walk through
    for name1 in names_1:
        for name2 in names_2:
            directory = root_path + '/' + name1 + '/' + name2
            analyze_a_type(directory)

    # for dir in os.listdir(root_path):
    #     if not os.path.isdir(os.path.join(root_path, dir)):
    #         continue
    #     if dir == 'update_distribution_influence':
    #         continue
    #     sub_path = os.path.join(root_path, dir)
    #     for subdir in os.listdir(sub_path):
    #         if not os.path.isdir(os.path.join(sub_path, subdir)):
    #             continue
    #         analyze_a_type(os.path.join(sub_path, subdir))

def collect_depth_3():
    root_path = 'workspace/edbt/file_size'
    names_1 = ['16M', '32M', '64M', '128M']
    names_2 = ['4', '6', '8', '10']
    names_3 = ['100_0', '50_50']

    for name1 in names_1:
        for name2 in names_2:
            for name3 in names_3:
                directory = root_path + '/' + name1 + '/' + name2 + '/' + name3
                analyze_a_type(directory)

def main():
    collect_depth_1()
    # collect_depth_2()
    # collect_depth_3()

if __name__ == '__main__':
    main()