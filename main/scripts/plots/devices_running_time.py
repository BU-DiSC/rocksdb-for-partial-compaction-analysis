import pandas as pd
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

matplotlib.rcParams['text.antialiased'] = True

fig, axs = plt.subplots(1, 2, figsize=(6, 3))

def get_data(df, strategy):
    data = df[df['Strategy'] == strategy]
    return data

def myplot(ax, strategy, word, i):
    data_root_path = 'workspace/edbt/compare_devices'
    labels = ['10', '30', '50']
    names = ['90_10', '70_30', '50_50']
    subset1 = []
    subset2 = []

    ax.text(0.95, 0.95, word, transform=ax.transAxes, fontsize=12, verticalalignment='top', horizontalalignment='right')

    for c in names:
        df = pd.read_csv(data_root_path + "/" + c + '/nvme1/result.txt', sep='\t')
        df = df[df['Strategy'] == strategy]
        avg = df['duration(us)']/1000000
        subset1.append(avg.values[0])
        df = pd.read_csv(data_root_path + "/"  + c + '/ssd/result.txt', sep='\t')
        df = df[df['Strategy'] == strategy]
        avg = df['duration(us)']/1000000
        subset2.append(avg.values[0])

    print(subset1)
    print(subset2)

    barWidth = 0.2
    r1 = np.arange(len(subset1))
    r2 = [x + barWidth for x in r1]

    if i == 0:
        ax.bar(r1, subset1, color='white', width=barWidth, edgecolor='black', label='PCIe SSD', linestyle='-')
        ax.bar(r2, subset2, color='white', width=barWidth, edgecolor='black', label='SSD', hatch='\\\\', linestyle='-')
    else:
        ax.bar(r1, subset1, color='white', width=barWidth, edgecolor='black', linestyle='-')
        ax.bar(r2, subset2, color='white', width=barWidth, edgecolor='black', hatch='\\\\', linestyle='-')

    ax.set_ylim(0, 200)
    ax.set_xlim(-0.5+barWidth/2, len(subset1) - 0.5+barWidth/2)

    ax.set_xlabel('Update proportion (%)')
    ax.set_xticks([r + barWidth/2 for r in range(len(labels))], labels)
    if i == 0:
        ax.set_ylabel('Running Time (s)')
    ax.set_yticks([50, 100, 150, 200])


save_path = 'figures/Device-Running-Time.pdf'

myplot(axs[0], 'kRoundRobin','(a)', 0)
myplot(axs[1], 'kMinOverlappingRatio','(b)', 1)

fig.subplots_adjust(bottom=0.2)
fig.subplots_adjust(top=0.87)

fig.legend(loc='upper center', 
           bbox_to_anchor=(0.5, 0.97), 
           ncol=4, 
           borderaxespad=0.0, 
           frameon=False,
           handletextpad=0.5,
           borderpad=0.3,
           labelspacing=0.2)

plt.savefig(save_path, dpi=600)
