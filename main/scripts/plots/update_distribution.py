import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

matplotlib.rcParams['text.antialiased'] = True

fig, axs = plt.subplots(2, 3, figsize=(12, 5))

def draw_explanation(ax, start_x, tick_width, first_seg_length, second_seg_length, val, rise_height, lab):
    ax.plot([start_x + tick_width, start_x + tick_width + first_seg_length], [val, val], color='black', linestyle='dashed', linewidth=1)
    ax.plot([start_x + tick_width + first_seg_length, start_x + tick_width + first_seg_length], [val, val + rise_height], color='black', linestyle='dashed', linewidth=1)
    ax.plot([start_x + tick_width + first_seg_length, start_x + tick_width + second_seg_length], [val + rise_height, val + rise_height], color='black', linestyle='dashed', linewidth=1)
    ax.scatter(start_x + tick_width + second_seg_length, val + rise_height, s=15, marker='>', color='black', linewidth=0.75)
    ax.text(start_x + tick_width + second_seg_length + 0.05, val + rise_height, lab, fontsize=10, verticalalignment='center', horizontalalignment='left')

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

    means = {}

    for i in range(len(data)):
        for j in range(len(strategies)):
            strategy = strategies[j]
            df = get_data(data[i], strategy)
            min_val = df['Min'] / data_total_bytes[i]
            max_val = df['Max'] / data_total_bytes[i]
            mean_val = df['Mean'] / data_total_bytes[i]
            percentile25val = df['25th'] / data_total_bytes[i]
            percentile75val = df['75th'] / data_total_bytes[i]
            start_x = i+strategy_wide*j

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
            means[strategy] = mean_val.values[0]

            if strategy == 'kOldestSmallestSeqFirst' and i == 0 and j == 3 and index == 0:
                rise_height = 0.30
                first_seg_length = 0.12
                second_seg_length = first_seg_length + 0.15
                val = max_val
                lab = 'max'
                draw_explanation(ax, start_x, tick_width, first_seg_length, second_seg_length, val, rise_height, lab)

                rise_height = 0.18
                first_seg_length = 0.18
                second_seg_length = first_seg_length + 0.15
                val = percentile75val
                lab = 'P75'
                draw_explanation(ax, start_x, tick_width, first_seg_length, second_seg_length, val, rise_height, lab)
                
                rise_height = 0.00
                first_seg_length = 0.15
                second_seg_length = first_seg_length + 0.8
                val = mean_val
                lab = 'mean'
                draw_explanation(ax, start_x, tick_width, first_seg_length, second_seg_length, val, rise_height, lab)

                rise_height = -0.15
                first_seg_length = 0.18
                second_seg_length = first_seg_length + 0.15
                val = percentile25val
                lab = 'P25'
                draw_explanation(ax, start_x, tick_width, first_seg_length, second_seg_length, val, rise_height, lab)

                rise_height = -0.80
                first_seg_length = 0.12
                second_seg_length = first_seg_length + 0.15
                val = min_val
                lab = 'min'
                draw_explanation(ax, start_x, tick_width, first_seg_length, second_seg_length, val, rise_height, lab)

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

    # Set the range of y-axis
    ax.set_ylim(y_start, y_end)
    ax.set_yticks(ticks=y_ticks)

    if index < 3:
        ax.xaxis.set_major_locator(ticker.NullLocator())

save_path = 'figures/Update-Distribution.pdf'
data_root = 'workspace/edbt/compare_distribution'

labels = ['10', '30', '50']
sub_dirs = ['90_10_0', '70_30_0', '50_50_0']
total_bytes = 5 * 1024 * 1024 * 1024
data_total_bytes = [total_bytes] * len(sub_dirs)

data_dirs = [data_root + '/update_normal1',
             data_root + '/update_normal2',
             data_root + '/update_normal3',
             data_root + '/update_zipfian1',
             data_root + '/update_zipfian2',
             data_root + '/update_zipfian3']
ax_index = [(0, 0), (0, 1), (0, 2), (1, 0), (1, 1), (1, 2)]
words = ['(a)', '(b)', '(c)', '(d)', '(e)', '(f)']
for i in range(len(data_dirs)):
    data_dir = data_dirs[i]
    myplot(data_dir, labels, sub_dirs, data_total_bytes, axs[ax_index[i]], i, words[i])

plt.subplots_adjust(hspace=0.10)

fig.legend(loc='upper center', 
           bbox_to_anchor=(0.5, 0.95), 
           ncol=len(fig.axes), 
           borderaxespad=0.0, 
           frameon=False,
           handletextpad=0.5,
           borderpad=0.3,
           labelspacing=0.2)

plt.savefig(save_path, dpi=600)

