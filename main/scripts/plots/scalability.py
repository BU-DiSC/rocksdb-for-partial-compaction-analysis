import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

matplotlib.rcParams['text.antialiased'] = True
fig, axs = plt.subplots(2, 3, figsize=(12, 5))

def get_data(df, strategy):
    data = df[df['Strategy'] == strategy]
    return data

def myplot(data_dir, labels, sub_dirs, data_total_bytes, ax, index, word, y_start, y_end, y_ticks):
    data = []
    for dir in sub_dirs:
        data.append(pd.read_csv(data_dir + '/' + dir + '/result.txt', sep='\t'))

    tick_width = 0.04

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
            if i == 0 and index == 4:
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
    if index >= 3:
        ax.set_xlabel('Update proportion (%)')

    # Set the x-axis labels
    offset = 0.24
    xticks = [i for i in range(len(labels))]
    for i in range(len(xticks)):
        xticks[i] = xticks[i] + offset
    ax.set_xticks(xticks, labels)
    ax.set_xlim(-0.5+0.24, len(labels)-0.5+0.24)

    ax.set_ylim(y_start, y_end)
    ax.set_yticks(ticks=y_ticks)

    if index < 3:
        ax.xaxis.set_major_locator(ticker.NullLocator())

save_path = 'figures/Scalability.pdf'
data_root = 'workspace/edbt/compare_workload_size'

labels = ['0', '25', '50']
sub_dirs = ['100_0', '75_25', '50_50']
total_bytes = 5 * 1024 * 1024 * 1024

names = ['20_512_size', '40_512_size', '80_512_size', '10_1024_size', '20_1024_size', '40_1024_size']
data_dirs = [data_root + '/' + n for n in names]
data_total_bytes = [[total_bytes*2 for i in range(len(labels))],
                    [total_bytes*4 for i in range(len(labels))],
                    [total_bytes*8 for i in range(len(labels))],
                    [total_bytes*2 for i in range(len(labels))],
                    [total_bytes*4 for i in range(len(labels))],
                    [total_bytes*8 for i in range(len(labels))]]
ax_index = [(0, 0), (0, 1), (0, 2), (1, 0), (1, 1), (1, 2)]
words = ['(a)', '(b)', '(c)', '(d)', '(e)', '(f)']

y_range = [(3, 6, np.arange(3, 6.1, 1)), (4, 7, np.arange(4, 7.1, 1)), (5, 8, np.arange(5, 8.1, 1)),
           (3, 6, np.arange(3, 6.1, 1)), (4, 7, np.arange(4, 7.1, 1)), (5, 8, np.arange(5, 8.1, 1))]
for i in range(len(data_dirs)):
    data_dir = data_dirs[i]
    y_start, y_end, y_ticks = y_range[i]
    myplot(data_dir, labels, sub_dirs, data_total_bytes[i], axs[ax_index[i]], i, words[i], y_start, y_end, y_ticks)

plt.subplots_adjust(hspace=0.1)

fig.legend(loc='upper center', 
           bbox_to_anchor=(0.5, 0.95), 
           ncol=len(fig.axes), 
           borderaxespad=0.0, 
           frameon=False,
           handletextpad=0.5,
           borderpad=0.3,
           labelspacing=0.2)

plt.savefig(save_path, dpi=600)
