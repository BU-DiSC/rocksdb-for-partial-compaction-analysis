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

def myplot(ax, name, word):
    data_root_path = 'workspace/edbt/instability_mor/' + name
    names = []
    for j in range(1, 51):
        names.append('run' + str(j))

    data = []

    ax.text(0.97, 0.97, word, transform=ax.transAxes, fontsize=12, verticalalignment='top', horizontalalignment='right')

    for n in names:
        df = pd.read_csv(data_root_path + "/" + n + '/result.txt', sep='\t')
        df = df[df['Strategy'] == 'kRefinedMOR']
        wa = df['WA']
        data.append(wa.values[0])

    threshold = 0.01
    data_label = [0] * len(data)
    label = 1
    data_type = []
    for j in range(len(data)):
        if data_label[j] == 0:
            for k in range(j+1, len(data)):
                if abs(data[j] - data[k]) < threshold:
                    data_label[k] = label
            data_label[j] = label
            label += 1
            data_type.append(round(data[j]+0.005, 2))
    
    data_count = {}
    for j in range(len(data)):
        if data_label[j] in data_count:
            data_count[data_label[j]] += 1
        else:
            data_count[data_label[j]] = 1    

    data_count = dict(sorted(data_count.items(), key=lambda item: item[1], reverse=True))
    data_count = dict(list(data_count.items())[:5])
    labels = [data_type[key-1] for key in data_count.keys()]
    sizes = list(data_count.values())

    ax.pie(sizes, labels=labels, autopct='%d%%', startangle=90)


save_path = 'figures/Instability-Of-RefinedMOR.pdf'

myplot(axs[0], '8000000_8/100_0_0','(a)')
myplot(axs[1], '2000000_64/100_0_0','(b)')

fig.subplots_adjust(bottom=0.2)
fig.subplots_adjust(top=0.87)

plt.savefig(save_path, dpi=600)
