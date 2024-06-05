import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

matplotlib.rcParams['text.antialiased'] = True
fig, axs = plt.subplots(1, 3, figsize=(6, 3))

def get_data(df, strategy):
    data = df[df['Strategy'] == strategy]
    return data

def myplot(data_dir, labels, sub_dirs, data_total_bytes, ax, index, word):
    data = []
    for dir in sub_dirs:
        data.append(pd.read_csv(data_dir + '/' + dir + '/result.txt', sep='\t'))

    tick_width = 0.04
    y_start = 2
    y_end = 5
    y_ticks = np.arange(y_start, y_end + 0.1, 1)

    strategies = ['kRoundRobin', 'kMinOverlappingRatio', 'kOldestLargestSeqFirst', 'kOldestSmallestSeqFirst']
    strategies_label = ['RoundRobin', 'MinOverlappingRatio', 'OldestLargestSeqFirst', 'OldestSmallestSeqFirst']
    strategy_wide = 0.16
    strategy_colors = ['red', 'blue', 'green', 'purple']

    ax.text(0.95, 0.95, word, transform=ax.transAxes, fontsize=12, verticalalignment='top', horizontalalignment='right')

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
            if i == 0 and index == 0:
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
        if i != len(data) - 1:
            # add a split dashed line
            ax.plot([i + 0.74, i + 0.74], [y_start, y_end], color='black', linestyle='dashed', linewidth=1)

    if index % 3 == 0:
        ax.set_ylabel('Write amplification')
    ax.set_xlabel('Device')

    # Set the x-axis labels
    offset = 0.24
    xticks = [i for i in range(len(labels))]
    for i in range(len(xticks)):
        xticks[i] = xticks[i] + offset
    ax.set_xticks(xticks, labels)
    ax.set_xlim(-0.5+0.24, len(labels)-0.5+0.24)

    # Set the range of y-axis
    ax.set_yticks(ticks=y_ticks)
    ax.set_ylim(y_start, y_end)

    # Remove y tick and label
    if index != 0:
        ax.yaxis.set_major_locator(ticker.NullLocator())

save_path = 'figures/Devices.pdf'

data_root = 'workspace/edbt/compare_devices'

labels = ['PCIe SSD', 'SSD']
sub_dirs = ['nvme1', 'ssd']
total_bytes = 5 * 1024 * 1024 * 1024
data_total_bytes = [total_bytes, total_bytes]

data_dirs = [data_root + '/90_10',
             data_root + '/70_30',
             data_root + '/50_50']
data_total_bytes = [[total_bytes, total_bytes] for i in range(len(data_dirs))]
words = ['(a)', '(b)', '(c)']
for i in range(len(data_dirs)):
    data_dir = data_dirs[i]
    myplot(data_dir, labels, sub_dirs, data_total_bytes[i], axs[i], i, words[i])

fig.subplots_adjust(bottom=0.2)
fig.subplots_adjust(top=0.86)
fig.subplots_adjust(wspace=0.05)

fig.legend(loc='upper center', 
           bbox_to_anchor=(0.5, 0.99), 
           ncol=2, 
           borderaxespad=0.0, 
           frameon=False,
           handletextpad=0.5,
           borderpad=0.3,
           labelspacing=0.2)

plt.savefig(save_path, dpi=600)

