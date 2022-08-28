#!/usr/bin/python2

# input: 	cartelle energymrtk/0/ ... energymrtk/9 e mrtk/0 ... mrtk/9 con all'interno i dump dei consumi
# 			per ogni core da t=0 a t=1000.
# output:	metriche di consumo per energymrtk e per mrtk: media per ogni t (=per righe) e somma dei consumi
#			little e big, oltre a min e max per isola

import glob
import re
import os
import sys
from copy import copy

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import json

FOLDER_MRTK = 'consumptions/mrtk/'
FOLDER_ENERGYMRTK = 'consumptions/energymrtk/'
FOLDER_ENERGYMRTK_BF = 'consumptions/energymrtk_bf/'
FOLDER_ENERGYMRTK_FF = 'consumptions/energymrtk_ff/'
FOLDER_GRUBPA = 'consumptions/grub_pa/'
TICKS_NO = 6000  # this parameter can be deleted with some code modifications
TICK_LIMIT = TICKS_NO


def process(folder, isPrint=True):
	"""
	Process 1 experiment only.
	Just change getStatistics() and getListConsumptions() as needed
	"""
	global TICK_LIMIT
	if not isPrint:
		sys.stdout = open(os.devnull, 'w')
	sumL = 0.0  # sum of avg cons. of little island
	sumB = 0.0
	avgValueL = 0.0
	avgValueB = 0.0
	maxValueL = 0.0
	maxValueB = 0.0
	minValueL = 0.0
	minValueB = 0.0
	sumValueL = {}
	sumValueB = {}
	files_energymrtk = getFilesInFolder(folder)
	#print (files_energymrtk)

	# consider 1 experiment and take the consumptions for each little and then big
	files_energymrtk_littles = getFilesWith(
		files_energymrtk, ["power", "LITTLE"])
	files_energymrtk_bigs = getFilesWith(files_energymrtk, ["power", "BIG"])

	print("{}, {}".format(files_energymrtk_littles, files_energymrtk_bigs))

#	TICK_LIMIT = 1000000000  # 1000 ms

	avgValueCore = {}  # file -> avg value
	for f in files_energymrtk_littles:
		avgValueCore[f], maxValueCore, minValueCore, totalEnergyCore = getStatistics(
			f)
		sumL += totalEnergyCore
		#if maxValueL < maxValueCore:
		#    maxValueL = maxValueCore
		#if minValueL > minValueCore:
		#    minValueL = minValueCore
	for f in files_energymrtk_bigs:
		avgValueCore[f], maxValueCore, minValueCore, totalEnergyCore = getStatistics(
			f)
		#print totalEnergyCore
		sumB += totalEnergyCore
		#if maxValueB < maxValueCore:
		#    maxValueB = maxValueCore
		#if minValueB > minValueCore:
		#    minValueB = minValueCore

	for f in files_energymrtk_littles:
		avgValueL += avgValueCore[f]
	for f in files_energymrtk_bigs:
		avgValueB += avgValueCore[f]
	avgValueL /= len(files_energymrtk_littles)
	avgValueB /= len(files_energymrtk_bigs)

	if not isPrint:
		sys.stdout = sys.__stdout__
	
	# sumValueB = {}: tick -> sum of consumptions on big cores
	return avgValueL, int(sumL), sumValueL, avgValueB, int(sumB), sumValueB


def getStatistics(filename):
	avgValue = 0
	maxValue = 0
	minValue = 1000
	energy = 0.0
	oldenergy = 0.0  # dbg
	o = 0
			
	content  = ''
	with open(filename, 'r') as f:
		content = f.readlines()

	for i in xrange(0, len(content) - 1):
		line_i = content[i].replace('Cur W=','').replace(' idle','').replace(' bzip2','').replace('t=', '')
		line_iplus1 = content[i+1].replace('Cur W=','').replace(' idle','').replace(' bzip2','').replace('t=', '')
		s_i = line_i.split(' ')
		s_iplus1 = line_iplus1.split(' ')

		w_i = float(s_i[0])
		t_i = int(s_i[1])
		w_iplus1 = float(s_iplus1[0])
		t_iplus1 = int(s_iplus1[1])

		assert t_iplus1 >= t_i  # time doesn't go back
		assert t_iplus1 >= 0 and t_i >= 0  # valid params
		assert w_i >= 0.0 and w_iplus1 >= 0.0

		energy += (t_iplus1 - t_i) * w_i
		assert energy >= oldenergy  # is energy increasing, right?
		oldenergy = energy
		# print("(t1-t0)*W => e: ({}-{})*{} => {}".format(t_iplus1, t_i, w_i, energy))

		if w_i < minValue:
			minValue = w_i
		if w_i > maxValue:
			maxValue = w_i

		o += 1  # num read lines

	avgValue = energy / TICK_LIMIT

	return avgValue, maxValue, minValue, energy

# utility functions


def getFoldersInForlder(folder):
	return os.listdir(folder)


def getFilesInFolder(folder):
	return glob.glob(folder + '*.txt')  # att, ~/folder/*.txt non funziona


def getFilesWith(paths, substrs, caseSensitive=False):
	res = []

	for p in paths:
		found = True
		for substr in substrs:
			if copy(substr).lower() not in copy(p).lower():
				found = False
		if found:
			res.append(p)

	return res


def updateTicksNo(filename):
	global TICKS_NO, TICK_LIMIT
	stringa = os.popen("head -n 325 {}".format(filename)).read()
	m = re.search("simulation_steps=(\d+)", stringa)
	TICKS_NO = int(m.groups()[0])
	TICK_LIMIT = TICKS_NO
	print("ticks are " + str(TICKS_NO))


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

	print(x)
	print(len(x))
	print(len(x) % 10)
	plt.xticks(np.arange(min(x), max(x)+1, len(x) / 10.0), rotation='vertical')

	plt.xlabel('time (ms)')
	plt.ylabel('W')
	plt.legend(loc='upper right')


def packGraph(filename='', isShow=False):
	if filename is not '':
		plt.savefig(filename, bbox_inches="tight")

	if isShow:
		plt.show()


def completeDictionaryWithMissingTicks(dd, untilKey=-1):
	# adds the missing ticks to a dictionary. The corresponding y will be the last found one
	# e.g., dd={ 0: 5, 3: 7, 5: 1 } => { 0: 5, 1: 5, 2: 5, 3: 7, 4: 7, 5: 1 }
	assert len(dd) > 2

	res = {}
	lastY = -1.0
	for i in xrange(0, len(dd.keys())):
		key1 = dd.keys()[i]
		key2 = dd.keys()[i+1]
		lastY = dd[key1]
		if key1 > untilKey:
			break
		for tick in xrange(key1, key2 + 1):
			res[tick] = lastY
	return res


def trimDictionary(dd, keysNo):
	# trims a dictionary. dictionary does't have to have
	# every single key, as in the example.
	# You must not do: dd = { ... }; dd = trimDictionary(dd, 100), but only trimDictionary(dd, 100)
	# e.g., dd={ 0: 5, 2: 5, 3: 7, 4: 7, 5: 1 }, keysNo=1 => { 0: 5 } (it has len(res)=1 key only)

	print("len before trim {}".format(len(dd)))
	to_remove = dd.keys()[keysNo:]
	for key in to_remove:
		del dd[key]
	print("len after trim {}".format(len(dd)))
	return dd


def trimDictionaryFromKeyOn(dd, key):
	dd = sorted(dd)
	i = len(dd)
	for k in dd.keys():
		if k >= key:
			break
		i += 1

	trimDictionary(dd, i)
	return dd


def printDictionaryToFile(dd, filename):
	if os.path.exists(filename):
		os.remove(filename)

	with open(filename, 'w') as file:
		for k in sorted(dd.keys()):
			file.write("%s %s\n" % (k, dd[k]))


def printNumpyNdarrayToFile(dd, filename):
	if os.path.exists(filename):
		os.remove(filename)

	s = ""
	for row in dd:
		for col in row:
			s += str(col) + " "
		s += '\n'

	with open(filename, 'w') as file:
		file.write(s)


def dictToMatrix(dd):
	# from https://stackoverflow.com/questions/15579649/python-dict-to-numpy-structured-array
	# result = {0: 1.1181753789488595, 1: 0.5566080288678394, 2: 0.4718269778030734, 3: 0.48716683119447185, 4: 1.0, 5: 0.1395076201641266, 6: 0.20941558441558442}
	# names = ['id','data']
	# formats = ['f8','f8']
	# dtype = dict(names = names, formats=formats)
	# array = np.array(list(result.items()), dtype=dtype)
	# yields
	# array([(0.0, 1.1181753789488595), (1.0, 0.5566080288678394),
	#       (2.0, 0.4718269778030734), (3.0, 0.48716683119447185), (4.0, 1.0),
	#       (5.0, 0.1395076201641266), (6.0, 0.20941558441558442)],
	#      dtype=[('id', '<f8'), ('data', '<f8')])
	result = {}
	names = ['id', 'data']
	formats = ['int64', 'f8']
	dtype = dict(names=names, formats=formats)
	result = np.fromiter(dd.iteritems(), dtype=dtype, count=len(dd))
	return result


def computeAvgArray(array):
	# returns the average of an array. e.g., array = [0, 8, 4, 6] -> computeAvgArray(array) = 9
	avg = 0.0
	for i in range(0, len(array)):
		avg += array[i]
	avg = avg / len(array)
	return avg


def computeSumDictionaries(dicts):
	# dictionaries list: dictionaries list, each one of type whatever -> double or int: { 0: 0.2332, 1: 0.3233 }
	# e.g., dicts = [ { 0: 0.4, 1: 2.2 }, { 0: 8.0, 1: 8.2 } ] -> computeAvgDictionaries(dicts) = { 0: 8.4, 1: 10.4 }
	sumDict = {}  # whatever -> avg of the doubles or ints

	min_ticks = len(dicts[0])
	for i in range(1, len(dicts)):
		if len(dicts[i]) < min_ticks:
			min_ticks = len(dicts[i])

	print ("You gave " + str(len(dicts)))
	for curTime in range(0, min_ticks):  # for each time
		sumDict[curTime] = 0.0  # otherwise you get KeyErrors
		for i in range(0, len(dicts)):  # for each dictionary
			sumDict[curTime] += dicts[i][curTime]

	print('Sum computed')
	return sumDict


def computeAvgDictionaries(dicts):
	# dictionaries list: dictionaries list, each one of type whatever -> double or int: { 0: 0.2332, 1: 0.3233 }
	# e.g., dicts = [ { 0: 0.4, 1: 2.2 }, { 0: 8.0, 1: 8.2 } ] -> computeAvgDictionaries(dicts) = { 0: 4.2, 1: 5.2 }
	avgDict = {}  # whatever -> avg of the doubles or ints

	min_ticks = len(dicts[0])
	for i in range(1, len(dicts)):
		if len(dicts[i]) < min_ticks:
			min_ticks = len(dicts[i])

	print ("You gave " + str(len(dicts)))
	for curTime in range(0, min_ticks):  # for each time
		avgDict[curTime] = 0.0  # otherwise you get KeyErrors
		for i in range(0, len(dicts)):  # for each dictionary
			avgDict[curTime] += dicts[i][curTime]
		avgDict[curTime] /= len(dicts)

	print('Average computed')
	return avgDict


def computeAvg(avgValueL, maxValueL, minValueL, sumL, avgValueB, maxValueB, minValueB, sumB):
	"""
	Given the average of consumption per experiment, now you compute the average among all of them, per island
	"""
	print('computing averages...')
	avgValueL_aux = {}
	avgValueB_aux = {}
	maxValueL_aux = 0.0
	minValueL_aux = 0.0
	maxValueB_aux = 0.0
	minValueB_aux = 0.0
	sumL_aux = 0.0
	sumB_aux = 0.0

	maxValueL_aux = computeAvgArray(maxValueL)
	minValueL_aux = computeAvgArray(minValueL)
	sumL_aux = computeAvgArray(sumL)
	maxValueB_aux = computeAvgArray(maxValueB)
	minValueB_aux = computeAvgArray(minValueB)
	sumB_aux = computeAvgArray(sumB)

	avgValueB_aux = computeAvgDictionaries(avgValueB)
	avgValueL_aux = computeAvgDictionaries(avgValueL)

	return avgValueL_aux, maxValueL_aux, minValueL_aux, sumL_aux, avgValueB_aux, maxValueB_aux, minValueB_aux, sumB_aux


# main

if __name__ == "__main__":
	avgValueL_EMRTK = {}
	maxValueL_EMRTK = {}
	minValueL_EMRTK = {}
	sumL_EMRTK = {}
	avgValueB_EMRTK = {}
	maxValueB_EMRTK = {}
	minValueB_EMRTK = {}
	sumB_EMRTK = {}
	avgValueL_MRTK = {}
	maxValueL_MRTK = {}
	minValueL_MRTK = {}
	sumL_MRTK = {}
	avgValueB_MRTK = {}
	maxValueB_MRTK = {}
	minValueB_MRTK = {}
	sumB_MRTK = {}
	avgValueL_GRUBPA = {}
	maxValueL_GRUBPA = {}
	minValueL_GRUBPA = {}
	sumL_GRUBPA = {}
	avgValueB_GRUBPA = {}
	maxValueB_GRUBPA = {}
	minValueB_GRUBPA = {}
	sumB_GRUBPA = {}
	sumB_eachTick_EMRTK = {}  # tick -> sum of consumptions of big cores
	sumL_eachTick_EMRTK = {}
	sumB_eachTick_MRTK = {}
	sumL_eachTick_MRTK = {}
	sumB_eachTick_GRUBPA = {}
	sumL_eachTick_GRUBPA = {}

	print('graph.py, found # experiments: ' +
		  str(len(getFoldersInForlder(FOLDER_ENERGYMRTK))))
	updateTicksNo(FOLDER_ENERGYMRTK + '0/log_emrtk_1_run.txt')
	avgValueL_EMRTK, energyL_EMRTK, sumL_eachTick_EMRTK, avgValueB_EMRTK, energyB_EMRTK, sumB_eachTick_EMRTK = process(FOLDER_ENERGYMRTK + '0/')
	avgValueL_MRTK, energyL_MRTK, sumL_eachTick_MRTK, avgValueB_MRTK, energyB_MRTK, sumB_eachTick_MRTK = process(FOLDER_MRTK + '0/')
	avgValueL_GRUBPA, energyL_GRUBPA, sumL_eachTick_GRUBPA, avgValueB_GRUBPA, energyB_GRUBPA, sumB_eachTick_GRUBPA = process(FOLDER_GRUBPA + '0/')
	avgValueL_EMRTK_BF, energyL_EMRTK_BF, sumL_eachTick_EMRTK_BF, avgValueB_EMRTK_BF, energyB_EMRTK_BF, sumB_eachTick_EMRTK_BF = process(FOLDER_ENERGYMRTK_BF + '0/')
	avgValueL_EMRTK_FF, energyL_EMRTK_FF, sumL_eachTick_EMRTK_FF, avgValueB_EMRTK_FF, energyB_EMRTK_FF, sumB_eachTick_EMRTK_FF = process(FOLDER_ENERGYMRTK_FF + '0/')

	print('big energy consumption. emrtk VS mrtk VS grub-pa: ' +
		  str(energyB_EMRTK) + " VS " + str(energyB_MRTK) + " VS " + str(energyB_GRUBPA))

	print ('paper sum: ' + str(energyB_EMRTK + energyL_EMRTK))
	print ('EDF sum: ' + str(energyB_MRTK + energyL_MRTK))
	print ('GRUB-PA sum: ' + str(energyB_GRUBPA + energyL_GRUBPA))

	if energyB_EMRTK + energyL_EMRTK > energyB_GRUBPA + energyL_GRUBPA:
		print ("WARN, EMRTK consumes more than GRUBPA")
	if energyB_EMRTK + energyL_EMRTK > energyB_MRTK + energyL_MRTK:
		print ("WARN, EMRTK consumes more than MRTK")

	data = {
		'energy consumptions': {
			'sum energy consumptions big emrtk': energyB_EMRTK,
			'sum energy consumptions big emrtk_bf': energyB_EMRTK_FF,
			'sum energy consumptions big emrtk_ff': energyB_EMRTK_FF,
			'sum energy consumptions big mrtk': energyB_MRTK,
			'sum energy consumptions big grubpa': energyB_GRUBPA,
			'sum energy consumptions little emrtk': energyL_EMRTK,
			'sum energy consumptions little emrtk_bf': energyL_EMRTK_BF,
			'sum energy consumptions little emrtk_ff': energyL_EMRTK_FF,
			'sum energy consumptions little mrtk': energyL_MRTK,
			'sum energy consumptions little grubpa': energyL_GRUBPA,

			'paper alg big + little': energyL_EMRTK + energyB_EMRTK,
			'BF big + little': energyL_EMRTK_BF + energyB_EMRTK_BF,
			'FF big + little': energyL_EMRTK_FF + energyB_EMRTK_FF,
			'EDF big + little': energyL_MRTK + energyB_MRTK,
			'GRUBPA big + little': energyL_GRUBPA + energyB_GRUBPA,

			'avg energy consumption big emrtk': energyB_EMRTK / TICK_LIMIT,
			'avg energy consumption little emrtk': energyL_EMRTK / TICK_LIMIT,
			'avg energy consumption big emrtk_bf': energyB_EMRTK_BF / TICK_LIMIT,
			'avg energy consumption little emrtk_bf': energyL_EMRTK_BF / TICK_LIMIT,
			'avg energy consumption big emrtk_ff': energyB_EMRTK_FF / TICK_LIMIT,
			'avg energy consumption little emrtk_ff': energyL_EMRTK_FF / TICK_LIMIT,
			'avg energy consumption big mrtk': energyB_MRTK / TICK_LIMIT,
			'avg energy consumption little mrtk': energyL_MRTK / TICK_LIMIT,
			'avg energy consumption big grubpa': energyB_GRUBPA / TICK_LIMIT,
			'avg energy consumption little grubpa': energyL_GRUBPA / TICK_LIMIT,
		},
	}

	if os.path.exists("graph_py_output.json"):
		os.remove("graph_py_output.json")
	with open('graph_py_output.json', 'w') as outfile:
		json.dump(data, outfile, indent=4)
