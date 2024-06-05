import pandas as pd
import numpy as np

def get_data(df, strategy):
    data = df[df['Strategy'] == strategy]
    return data

data_root = 'workspace/edbt/compare_proportion'
sub_dirs = ['100_0_0', '90_10_0', '80_20_0', '70_30_0', '60_40_0', '50_50_0', '40_60_0', '30_70_0', '20_80_0', '10_90_0']
data = []
for dir in sub_dirs:
    data.append(pd.read_csv(data_root + '/' + dir + '/result.txt', sep='\t'))

strategies = ['kRoundRobin', 'kMinOverlappingRatio', 'kOldestLargestSeqFirst', 'kOldestSmallestSeqFirst']

for i in range(len(data)):
    # plot all strategies
    means = {}
    print("data: ", sub_dirs[i])
    for j in range(len(strategies)):
        strategy = strategies[j]
        df = get_data(data[i], strategy)
        mean_val = df['Mean']
        means[strategy] = mean_val.values[0]
        print("strategy: ", strategy, "mean: ", mean_val.values[0])
    # print("RR compared with OLSF: ", (means["kOldestSmallestSeqFirst"] - means["kRoundRobin"]) / means["kRoundRobin"] * 100)
    # print("MOR compared with OLSF: ", (means["kOldestSmallestSeqFirst"] - means["kMinOverlappingRatio"]) / means["kMinOverlappingRatio"] * 100)


