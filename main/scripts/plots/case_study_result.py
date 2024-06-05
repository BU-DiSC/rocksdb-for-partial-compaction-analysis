import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

matplotlib.rcParams['text.antialiased'] = True

fig, axs = plt.subplots(1, 3, figsize=(12, 3))

def get_data(df, strategy):
    data = df[df['Strategy'] == strategy]
    return data

def myplot(data_dir, labels, sub_dirs, data_total_bytes, ax, index, word, y_start, y_end, y_ticks):
    data = []
    for dir in sub_dirs:
        data.append(pd.read_csv(data_dir + '/' + dir + '/result.txt', sep='\t'))

    tick_width = 0.04

    strategies = ['kRoundRobin', 'kMinOverlappingRatio', 'kOldestLargestSeqFirst', 'kOldestSmallestSeqFirst', 'kRefinedMOR']
    strategies_label = ['RoundRobin', 'MinOverlappingRatio', 'OldestLargestSeqFirst', 'OldestSmallestSeqFirst', 'RefinedMOR']
    strategy_wide = 0.16
    strategy_colors = ['red', 'blue', 'green', 'purple', 'brown']

    ax.text(0.95, 0.95, word, transform=ax.transAxes, fontsize=12, verticalalignment='top', horizontalalignment='right')

    means = {}

    for i in range(len(data)):
        # plot all strategies
        for j in range(len(strategies)):
            strategy = strategies[j]
            df = get_data(data[i], strategy)
            min_val = df['Min'] / data_total_bytes[i]
            max_val = df['Max'] / data_total_bytes[i]
            mean_val = df['Mean'] / data_total_bytes[i]
            percentile25val = df['25th'] / data_total_bytes[i]
            percentile75val = df['75th'] / data_total_bytes[i]
            # Plot a line from low to high
            start_x = i+strategy_wide*j

            # Plot min and max
            if i == 0 and index == 1:
                ax.plot([start_x, start_x], [min_val, percentile25val], label=strategies_label[j], color=strategy_colors[j], linewidth=1)
            else:
                ax.plot([start_x, start_x], [min_val, percentile25val], color=strategy_colors[j], linewidth=1)
            ax.plot([start_x, start_x], [percentile75val, max_val], color=strategy_colors[j], linewidth=1)
            ax.plot([start_x - tick_width, start_x + tick_width], [min_val, min_val], color=strategy_colors[j], linewidth=1)
            ax.plot([start_x - tick_width, start_x + tick_width], [max_val, max_val], color=strategy_colors[j], linewidth=1)

            # Plot a box between 25th and 75th percentile
            ax.plot([start_x - tick_width, start_x + tick_width], [percentile25val, percentile25val], color=strategy_colors[j], linewidth=1)
            ax.plot([start_x - tick_width, start_x + tick_width], [percentile75val, percentile75val], color=strategy_colors[j], linewidth=1)
            ax.plot([start_x - tick_width, start_x - tick_width], [percentile25val, percentile75val], color=strategy_colors[j], linewidth=1)
            ax.plot([start_x + tick_width, start_x + tick_width], [percentile25val, percentile75val], color=strategy_colors[j], linewidth=1)

            # Plot mean
            ax.scatter(start_x, mean_val, s=15, marker='x', color=strategy_colors[j], zorder=10, linewidth=0.75)
            means[strategy] = mean_val.values[0]
        if i != len(data) - 1:
            ax.plot([i + 0.8, i + 0.8], [y_start, y_end], color='black', linestyle='dashed', linewidth=1)

    if index == 0:
        ax.set_ylabel('Write amplification')

    ax.set_xlabel('Update proportion (%)')

    offset = 0.31
    xticks = [i for i in range(len(labels))]
    for i in range(len(xticks)):
        xticks[i] = xticks[i] + offset
    ax.set_xticks(xticks, labels)

    ax.set_ylim(y_start, y_end)
    ax.set_yticks(ticks=y_ticks)

save_path = 'figures/Case-Study-Result-3.pdf'
data_root = 'workspace/edbt/compare_workload_size/refinedmor'

labels = ['10', '30', '50']
sub_dirs = ['90_10_0', '70_30_0', '50_50_0']
total_bytes = 5 * 1024 * 1024 * 1024
data_total_bytes = [total_bytes, total_bytes, total_bytes]
y_start = 2
y_end = 5
y_ticks = np.arange(y_start, y_end + 0.1, 1)
data_dir = data_root + '/update_normal1'
myplot(data_dir, labels, sub_dirs, data_total_bytes, axs[0], 0, '(a)', y_start, y_end, y_ticks)

labels = ['0', '25', '50']
sub_dirs = ['100_0', '75_25', '50_50']
total_bytes = 20 * 1024 * 1024 * 1024
data_total_bytes = [total_bytes, total_bytes, total_bytes]
y_start = 4
y_end = 7
y_ticks = np.arange(y_start, y_end + 0.1, 1)
data_dir = data_root + '/40_512_size'
myplot(data_dir, labels, sub_dirs, data_total_bytes, axs[1], 1, '(b)', y_start, y_end, y_ticks)

labels = ['0', '25', '50']
sub_dirs = ['100_0', '75_25', '50_50']
total_bytes = 40 * 1024 * 1024 * 1024
data_total_bytes = [total_bytes, total_bytes, total_bytes]
y_start = 5
y_end = 8
y_ticks = np.arange(y_start, y_end + 0.1, 1)
data_dir = data_root + '/40_1024_size'
myplot(data_dir, labels, sub_dirs, data_total_bytes, axs[2], 2, '(c)', y_start, y_end, y_ticks)

fig.subplots_adjust(bottom=0.2)
fig.subplots_adjust(top=0.87)

fig.legend(loc='upper center', 
           bbox_to_anchor=(0.5, 0.97), 
           ncol=5, 
           borderaxespad=0.0, 
           frameon=False,
           handletextpad=0.5,
           borderpad=0.3,
           labelspacing=0.2)

plt.savefig(save_path, dpi=600)
