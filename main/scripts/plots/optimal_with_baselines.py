import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

matplotlib.rcParams['text.antialiased'] = True

fig, axs = plt.subplots(1, 1, figsize=(6, 3.5))

def get_data(df, strategy):
    data = df[df['Strategy'] == strategy]
    return data

def myplot(data_dir, labels, sub_dirs, data_total_bytes, ax, index):
    data = []
    for dir in sub_dirs:
        data.append(pd.read_csv(data_dir + '/' + dir + '/result.txt', sep='\t'))

    tick_width = 0.04
    y_start = 2
    y_end = 5
    y_ticks = np.arange(y_start, y_end + 0.1, 1)

    strategies = ['kRoundRobin', 'kMinOverlappingRatio', 'kOldestLargestSeqFirst', 'kOldestSmallestSeqFirst', 'minimum']
    strategies_label = ['RoundRobin', 'MinOverlappingRatio', 'OldestLargestSeqFirst', 'OldestSmallestSeqFirst', 'Optimal']
    strategy_wide = 0.16
    strategy_colors = ['red', 'blue', 'green', 'purple', 'orange']

    for i in range(len(data)):
        # plot all strategies
        for j in range(len(strategies)):
            strategy = strategies[j]
            df = get_data(data[i], strategy)
            min_val = df['Min'] / data_total_bytes[i] + 1
            max_val = df['Max'] / data_total_bytes[i] + 1
            mean_val = df['Mean'] / data_total_bytes[i] + 1
            percentile25val = df['25th'] / data_total_bytes[i] + 1
            percentile75val = df['75th'] / data_total_bytes[i] + 1
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
            ax.plot([i + 0.81, i + 0.81], [y_start, y_end], color='black', linestyle='dashed', linewidth=1)

    ax.set_ylabel('Write amplification')
    ax.set_xlabel('Update proportion (%)')

    # Set the x-axis labels
    offset = 0.31
    xticks = [i for i in range(len(labels))]
    for i in range(len(xticks)):
        xticks[i] = xticks[i] + offset
    ax.set_xticks(xticks, labels)
    ax.set_xlim(-0.5+0.31, len(labels)-0.5+0.31)

    # Set the range of y-axis
    ax.set_ylim(y_start, y_end)
    ax.set_yticks(ticks=y_ticks)

save_path = 'figures/Optimal-With-Baselines.pdf'
data_root = 'workspace/compare_optimal_with_baselines'

labels = ['0', '10', '20', '30', '40', '50']
sub_dirs = ['100_0', '90_10', '80_20', '70_30', '60_40', '50_50']
total_bytes = 2500000 * 64
data_total_bytes = [total_bytes] * len(sub_dirs)

myplot(data_root, labels, sub_dirs, data_total_bytes, axs, 0)

plt.subplots_adjust(top=0.80)
plt.subplots_adjust(bottom=0.15)

fig.legend(loc='upper center', 
           bbox_to_anchor=(0.5, 0.98), 
           ncol=2, 
           borderaxespad=0.0, 
           frameon=False,
           handletextpad=0.5,
           borderpad=0.3,
           labelspacing=0.2)

plt.savefig(save_path, dpi=600)

