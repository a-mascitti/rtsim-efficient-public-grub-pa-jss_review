#!/usr/bin/python3

import sys
import os
import re

FOLDER_EMRTK = 'consumptions/energymrtk/'
FOLDER_GRUBPA = 'consumptions/grub_pa/'
FOLDER_EMRTK_BF = 'consumptions/energymrtk_bf/'
FOLDER_EMRTK_FF = 'consumptions/energymrtk_ff/'

def ASSERT_ROUTINE(exp, msg=''):
    if not exp:
        print("Error, " + msg)
    assert(exp)


def process(folder):
    filenames = [
        folder + '0/powerBIG.txt',
        folder + '0/powerLITTLE.txt',
    ]

    powers_big = getStatistics(filenames[0])  # w -> sum tick
    powers_little = getStatistics(filenames[1])

    return powers_big, powers_little


def getStatistics(fn):
    res = {}
    with open(fn, 'r') as f:
        content = f.readlines()
    for i in range(0, len(content) - 1):
        line_i = content[i]
        line_iplus1 = content[i+1]
        s_i = line_i.split(' ')
        s_iplus1 = line_iplus1.split(' ')

        w_i = float(s_i[0])  # w of island
        t_i = int(s_i[1])
        w_iplus1 = float(s_iplus1[0])  # w of island
        t_iplus1 = int(s_iplus1[1])

        assert t_iplus1 >= t_i  # time doesn't go back
        assert t_iplus1 >= 0 and t_i >= 0  # valid params
        assert w_i >= 0.0 and w_iplus1 >= 0.0

        if w_i not in res.keys():
               res[w_i] = 0
        res[w_i] += t_iplus1 - t_i

    return res


def updateTicksNo(filename):
    stringa = os.popen("head -n 325 {}".format(filename)).read()
    m = re.search("simulation_steps=(\d+)", stringa)
    hyperp = int(m.groups()[0])
    return hyperp


def cdf(mappa):
    # mappa: W -> ns
    # res:   W -> [ ns / hyperp, ecdf ]
    res = {}
    keys = list(sorted(mappa.keys()))
    hyperp = sum(mappa.values())
    for i in range(0, len(keys)):
         k = keys[i]
         perc = mappa[k] / hyperp
         cdf  = (perc if i == 0 else perc + res[keys[i-1]][1])
         res[k] = [ perc, cdf ]
    assert(0.99 <= res[keys[len(keys)-1]][1] <= 1.2)
    res[keys[len(keys)-1]] = [ res[keys[len(keys)-1]][0], 1.0 ]
    return res


powers_big_emrtk, powers_little_emrtk = process(FOLDER_EMRTK)
powers_big_grubpa, powers_little_grubpa = process(FOLDER_GRUBPA)
powers_big_emrtk_bf, powers_little_emrtk_bf = process(FOLDER_EMRTK_BF)
powers_big_emrtk_ff, powers_little_emrtk_ff = process(FOLDER_EMRTK_FF)


# checks
hyperp = updateTicksNo(FOLDER_EMRTK + '0/log_emrtk_1_run.txt')
res = 0
for k in powers_big_emrtk:
    res += powers_big_emrtk[k]
ASSERT_ROUTINE(res == hyperp)
res = 0
for k in powers_little_emrtk:
    res += powers_little_emrtk[k]
ASSERT_ROUTINE(res == hyperp)
res = 0
for k in powers_big_grubpa:
    res += powers_big_grubpa[k]
ASSERT_ROUTINE(res == hyperp)
res = 0
for k in powers_little_grubpa:
    res += powers_little_grubpa[k]
ASSERT_ROUTINE(res == hyperp)
res = 0
for k in powers_big_emrtk_bf:
    res += powers_big_emrtk_bf[k]
ASSERT_ROUTINE(res == hyperp)
res = 0
for k in powers_little_emrtk_bf:
    res += powers_little_emrtk_bf[k]
ASSERT_ROUTINE(res == hyperp)
res = 0
for k in powers_big_emrtk_ff:
    res += powers_big_emrtk_ff[k]
ASSERT_ROUTINE(res == hyperp)
res = 0
for k in powers_little_emrtk_ff:
    res += powers_little_emrtk_ff[k]
ASSERT_ROUTINE(res == hyperp)

# write data to file
with open('pow_ticks_big_emrtk_output.dat','w') as f:
    f.write('W ns\n')
    keys = list(powers_big_emrtk.keys())
    keys = sorted(keys)
    _cdf = cdf(powers_big_emrtk)
    print(sorted(powers_big_emrtk))
    print('----------')
    print(_cdf)
    for k in keys:
        if k not in powers_big_emrtk.keys(): powers_big_emrtk[k] = 0
        if k not in powers_little_emrtk.keys(): powers_little_emrtk[k] = 0
        if powers_big_emrtk[k] != 0:
              f.write('{} {} {} {}\n'.format(k, powers_big_emrtk[k], _cdf[k][0], _cdf[k][1]))
with open('pow_ticks_little_emrtk_output.dat', 'w') as f:
    f.write('W ns\n')
    keys = list(powers_little_emrtk.keys())
    keys = sorted(keys)
    _cdf = cdf(powers_little_emrtk)
    for k in keys:
        if k not in powers_big_emrtk.keys(): powers_big_emrtk[k] = 0
        if k not in powers_little_emrtk.keys(): powers_little_emrtk[k] = 0
        if powers_little_emrtk[k] != 0:
              f.write('{} {} {} {}\n'.format(k, powers_little_emrtk[k], _cdf[k][0], _cdf[k][1]))
with open('pow_ticks_big_grubpa_output.dat','w') as f:
    f.write('W ns\n')
    keys = list(powers_big_grubpa.keys())
    keys = sorted(keys)
    _cdf = cdf(powers_big_grubpa)
    for k in keys:
        if k not in powers_big_grubpa.keys(): powers_big_grubpa[k] = 0
        if k not in powers_little_grubpa.keys(): powers_little_grubpa[k] = 0
        if powers_big_grubpa[k] != 0:
              f.write('{} {} {} {}\n'.format(k, powers_big_grubpa[k], _cdf[k][0], _cdf[k][1]))
with open('pow_ticks_little_grubpa_output.dat', 'w') as f:
    f.write('W ns ns/hyperp\n')
    keys = list(powers_little_grubpa.keys())
    keys = sorted(keys)
    _cdf = cdf(powers_little_grubpa)
    for k in keys:
        if k not in powers_big_grubpa.keys(): powers_big_grubpa[k] = 0
        if k not in powers_little_grubpa.keys(): powers_little_grubpa[k] = 0
        if powers_little_grubpa[k] != 0:
              f.write('{} {} {} {}\n'.format(k, powers_little_grubpa[k], _cdf[k][0], _cdf[k][1]))
with open('pow_ticks_big_emrtk_bf_output.dat','w') as f:
    f.write('W ns\n')
    keys = list(powers_big_emrtk_bf.keys())
    keys = sorted(keys)
    _cdf = cdf(powers_big_emrtk_bf)
    for k in keys:
        if k not in powers_big_emrtk_bf.keys(): powers_big_emrtk_bf[k] = 0
        if k not in powers_little_emrtk_bf.keys(): powers_little_emrtk_bf[k] = 0
        if powers_big_emrtk_bf[k] != 0:
              f.write('{} {} {} {}\n'.format(k, powers_big_emrtk_bf[k], _cdf[k][0], _cdf[k][1]))
with open('pow_ticks_little_emrtk_bf_output.dat', 'w') as f:
    f.write('W ns ns/hyperp\n')
    keys = list(powers_little_emrtk_bf.keys())
    keys = sorted(keys)
    _cdf = cdf(powers_little_emrtk_bf)
    for k in keys:
        if k not in powers_big_emrtk_bf.keys(): powers_big_emrtk_bf[k] = 0
        if k not in powers_little_emrtk_bf.keys(): powers_little_emrtk_bf[k] = 0
        if powers_little_emrtk_bf[k] != 0:
              f.write('{} {} {} {}\n'.format(k, powers_little_emrtk_bf[k], _cdf[k][0], _cdf[k][1]))
with open('pow_ticks_big_emrtk_ff_output.dat','w') as f:
    f.write('W ns\n')
    keys = list(powers_big_emrtk_ff.keys())
    keys = sorted(keys)
    _cdf = cdf(powers_big_emrtk_ff)
    for k in keys:
        if k not in powers_big_emrtk_ff.keys(): powers_big_emrtk_ff[k] = 0
        if k not in powers_little_emrtk_ff.keys(): powers_little_emrtk_ff[k] = 0
        if powers_big_emrtk_ff[k] != 0:
              f.write('{} {} {} {}\n'.format(k, powers_big_emrtk_ff[k], _cdf[k][0], _cdf[k][1]))
with open('pow_ticks_little_emrtk_ff_output.dat', 'w') as f:
    f.write('W ns ns/hyperp\n')
    keys = list(powers_little_emrtk_ff.keys())
    keys = sorted(keys)
    _cdf = cdf(powers_little_emrtk_ff)
    for k in keys:
        if k not in powers_big_emrtk_ff.keys(): powers_big_emrtk_ff[k] = 0
        if k not in powers_little_emrtk_ff.keys(): powers_little_emrtk_ff[k] = 0
        if powers_little_emrtk_ff[k] != 0:
              f.write('{} {} {} {}\n'.format(k, powers_little_emrtk_ff[k], _cdf[k][0], _cdf[k][1]))
