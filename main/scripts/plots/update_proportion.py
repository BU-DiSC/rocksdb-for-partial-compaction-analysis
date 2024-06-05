import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

matplotlib.rcParams['text.antialiased'] = True

def get_data(df, strategy):
    data = df[df['Strategy'] == strategy]
    return data

def myplot(data_dir, labels, sub_dirs, data_total_bytes):
    fig, ((ax1), (ax2)) = plt.subplots(1, 2, figsize=(6, 2.5))

    data = []
    for dir in sub_dirs:
        data.append(pd.read_csv(data_dir + '/' + dir + '/result.txt', sep='\t'))

    y_start = 2
    y_end = 5
    y_ticks = np.arange(y_start, y_end + 0.1, 1)

    strategies = ['kRoundRobin', 'kMinOverlappingRatio', 'kOldestLargestSeqFirst', 'kOldestSmallestSeqFirst']
    strategies_label = ['RR', 'MOR', 'OLSF', 'OSSF']
    strategy_colors = ['red', 'blue', 'green', 'purple']
    strategy_markers = ['^', 's', 'o', 'D']

    means = {k: [] for k in strategies}
    maxs = {k: [] for k in strategies}

    ax1.text(0.95, 0.95, '(a)', transform=ax1.transAxes, fontsize=12, verticalalignment='top', horizontalalignment='right')
    ax2.text(0.95, 0.95, '(b)', transform=ax2.transAxes, fontsize=12, verticalalignment='top', horizontalalignment='right')

    for i in range(len(data)):
        # plot all strategies
        for j in range(len(strategies)):
            strategy = strategies[j]
            df = get_data(data[i], strategy)
            mean_val = df['Mean'] / data_total_bytes[i]
            max_val = df['Max'] / data_total_bytes[i]
            means[strategy].append(mean_val.values[0])
            maxs[strategy].append(max_val.values[0])

    for i in range(len(strategies)):
        ax1.plot([i+1 for i in range(len(data))], means[strategies[i]], label=strategies_label[i], 
                 color=strategy_colors[i], linewidth=1, 
                 marker=strategy_markers[i],markersize=5, markeredgewidth=1, markerfacecolor='none', markeredgecolor=strategy_colors[i])
        ax2.plot([i+1 for i in range(len(data))], maxs[strategies[i]], 
                 color=strategy_colors[i], linewidth=1, 
                 marker=strategy_markers[i],markersize=5, markeredgewidth=1, markerfacecolor='none', markeredgecolor=strategy_colors[i])

    xticks = [i+1 for i in range(len(labels))]
    ax1.set_xticks(xticks, labels)
    ax2.set_xticks(xticks, labels)

    ax1.set_ylim(y_start, y_end)
    ax2.set_ylim(y_start, y_end)


    ax1.set_yticks(ticks=y_ticks)
    ax2.set_yticks(ticks=y_ticks)

    ax1.set_xlabel('Update proportion(%)')
    ax2.set_xlabel('Update proportion(%)')
    ax1.set_ylabel('Write amplification')

    plt.subplots_adjust(left=0.17)
    plt.subplots_adjust(bottom=0.22)
    plt.subplots_adjust(right=0.9)
    plt.subplots_adjust(top=0.85)

    fig.legend(loc='upper center', 
           bbox_to_anchor=(0.5, 0.95), 
           ncol=4, 
           borderaxespad=0.0, 
           frameon=False,
           handletextpad=0.5,
           borderpad=0.3,
           labelspacing=0.2)


save_path = 'figures/Update-Proportion.pdf'
data_root = 'workspace/edbt/compare_proportion'

labels = [None, '10', None, '30', None, '50', None, '70', None, '90']
sub_dirs = ['100_0_0', '90_10_0', '80_20_0', '70_30_0', '60_40_0', '50_50_0', '40_60_0', '30_70_0', '20_80_0', '10_90_0']
total_bytes = 5 * 1024 * 1024 * 1024
data_total_bytes = [total_bytes] * len(sub_dirs)

data_dir = data_root + "/proportion"
myplot(data_dir, labels, sub_dirs, data_total_bytes)

plt.savefig(save_path, dpi=600)
