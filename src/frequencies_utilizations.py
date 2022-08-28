#!/usr/bin/python2

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import json
import os
import sys

EXPS_NO = 10


def process(folder, isPrint=False):
    """
    Process 1 util only.
    Just change getStatistics() and getListConsumptions() as needed
    """
    global TICK_LIMIT
    if not isPrint:
        sys.stdout = open(os.devnull, 'w')

    files = [
        "avg_frequency_big_emrtk.dat",  # frequencies[0]
        "avg_frequency_little_emrtk.dat",
        "avg_frequency_big_grubpa.dat",
        "avg_frequency_little_grubpa.dat",  # [3]
        "avg_frequency_big_emrtk_bf.dat",
        "avg_frequency_little_emrtk_bf.dat",
        "avg_frequency_big_emrtk_ff.dat",  # [6]
        "avg_frequency_little_emrtk_ff.dat",
    ]

    frequencies = []
    for file in files:
        temp = []
        filename = folder + file
        with open(filename, 'r') as f:
            for line in f:
                temp.append(float(line.replace("\"", "")))
        #assert len(temp) == EXPS_NO
        frequencies.append(temp)

    means = []
    vars = []
    for frequencies_architecture in frequencies:
        means.append(np.mean(frequencies_architecture))
        vars.append(np.std(frequencies_architecture))

    if not isPrint:
        sys.stdout = sys.__stdout__

    assert len(means) == len(files)
    assert len(vars) == len(files)

    return means, vars


def printDictionaryToFile(dd, filename):
    if os.path.exists(filename):
        os.remove(filename)

    with open(filename, 'w') as file:
        for k in sorted(dd.keys()):
            file.write("%s %s\n" % (k, dd[k]))


# ------------------------------------------------ graphs

def newGraph():
    plt.figure()


# https://matplotlib.org/gallery/lines_bars_and_markers/line_styles_reference.html
def addToGraph(xy, label, linestyle='--'):
    # xy = { time -> voltage }
    x = list(xy.keys())
    y = list(xy.values())
    #print('x=%s' % str(x))
    #print('y=%s' % str(y))
    plt.plot(x, y, label=label, color='black', linestyle=linestyle)

    # print(x)
    # print(len(x))
    # print(len(x)%EXPERIMENTS_NO)
    plt.xticks(np.arange(min(x), max(x)+1, len(x) / EXPS_NO),
               rotation='vertical')

    plt.xlabel('time (ms)')
    plt.ylabel('W')
    plt.legend(loc='upper right')


def packGraph(filename='', isShow=False):
    if filename is not '':
        plt.savefig(filename, bbox_inches="tight")

    if isShow:
        plt.show()


if __name__ == "__main__":
    variances = []  # [ [avg freq big, avg freq little ] ]
    means = []

    # EXPS_NO=int(sys.argv[1])

    #utils = [ 0.25, 0.4, 0.5, 0.65 ]
    utils = [0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7]
    utils8=[u*8 for u in utils]
    for u in utils:
        m, v = process("run-" + str(u) + "/", True)
        print m
        means.append(m)
        variances.append(v)

    assert len(variances) == len(utils)
    assert len(means) == len(utils)

    plt.xlabel('Total utilization')
    plt.ylabel('Avg frequency (MHz)')
    plt.xticks(utils8)
    plt.grid()
    plotlittle = plt.errorbar(utils8, [means[i][1] for i in range(0, len(means))], [
        variances[i][1] for i in range(0, len(variances))], linestyle='-', marker='o', color='green')
    plotlittle_grubpa = plt.errorbar(utils8, [means[i][3] for i in range(0, len(means))], [
        variances[i][3] for i in range(0, len(variances))], linestyle='None', marker='o', color='black')
    plotlittle_bf = plt.errorbar(utils8, [means[i][5] for i in range(0, len(means))], [
        variances[i][1] for i in range(0, len(variances))], linestyle='None', marker='o', color='red')
    plotlittle_ff = plt.errorbar(utils8, [means[i][7] for i in range(0, len(means))], [
        variances[i][1] for i in range(0, len(variances))], linestyle='None', marker='o', color='blue', markersize=4)
    plt.legend([plotlittle, plotlittle_grubpa, plotlittle_bf, plotlittle_ff], [
        'LITTLE, BL-CBS', 'LITTLE, GRUB-PA', 'LITTLE, EDF-BF', 'LITTLE, EDF-FF'], loc='lower left', bbox_to_anchor=(0.0, 1.0), ncol=3)  # emrtk and GRUB-PA
    packGraph('frequencies_utilizations_little.pdf')

    plt.figure()
    plt.xlabel('Total utilization')
    plt.ylabel('Avg frequency (MHz)')
    plt.xticks(utils8)
    plt.grid()
    plotbig = plt.errorbar(utils8, [means[i][0] for i in range(0, len(means))], [
        variances[i][0] for i in range(0, len(variances))], linestyle='-', marker='*', color='green')
    plotbig_grubpa = plt.errorbar(utils8, [means[i][2] for i in range(0, len(means))], [
        variances[i][2] for i in range(0, len(variances))], linestyle='None', marker='*', color='black')
    plotbig_bf = plt.errorbar(utils8, [means[i][4] for i in range(0, len(means))], [
        variances[i][0] for i in range(0, len(variances))], linestyle='None', marker='*', color='red')
    plotbig_ff = plt.errorbar(utils8, [means[i][6] for i in range(0, len(means))], [
        variances[i][0] for i in range(0, len(variances))], linestyle='None', marker='*', color='blue', markersize=4)
    plt.legend([plotbig, plotbig_grubpa, plotbig_bf, plotbig_ff], [
        'big, BL-CBS', 'big, GRUB-PA', 'big, EDF-BF', 'big, EDF-FF'], loc='lower left', bbox_to_anchor=(0.0, 1.0), ncol=3)  # emrtk and GRUB-PA
    packGraph('frequencies_utilizations_big.pdf')

    data = {
        "avg frequency emrtk for utils " + str(utils): str(means[0]),
        "avg frequency emrtk (only bigs) for utils " + str(utils): str([means[i][0] for i in range(0, len(means))]),
        "avg frequency emrtk (only LITTLEs) for utils " + str(utils): str([means[i][1] for i in range(0, len(means))]),

        "avg frequency gubpa for utils " + str(utils): str(means[1]),
        "avg frequency gubpa (only bigs) for utils " + str(utils): str([means[i][2] for i in range(0, len(means))]),
        "avg frequency gubpa (only LITTLEs) for utils " + str(utils): str([means[i][3] for i in range(0, len(means))]),

        "avg frequency emrtk_bf for utils " + str(utils): str(means[2]),
        "avg frequency emrtk_bf (only bigs) for utils " + str(utils): str([means[i][4] for i in range(0, len(means))]),
        "avg frequency emrtk_bf (only LITTLEs) for utils " + str(utils): str([means[i][5] for i in range(0, len(means))]),

        "avg frequency emrtk_ff for utils " + str(utils): str(means[3]),
        "avg frequency emrtk_ff (only bigs) for utils " + str(utils): str([means[i][6] for i in range(0, len(means))]),
        "avg frequency emrtk_ff (only LITTLEs) for utils " + str(utils): str([means[i][7] for i in range(0, len(means))]),
    }

    filename = "frequencies_utilizations.dat"
    print("Data saved into file " + filename)
    printDictionaryToFile(data, filename)
