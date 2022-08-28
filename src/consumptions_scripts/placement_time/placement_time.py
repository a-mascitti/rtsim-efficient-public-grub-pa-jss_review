#!/usr/bin/python2

# input: 	cartelle energymrtk/0/ ... energymrtk/9 e mrtk/0 ... mrtk/9 con all'interno i dump dei consumi
# 			per ogni core da t=0 a t=1000
# output:	how much time the placement alg has spent for each experiment and the average among the experiments (sec)

import glob, re, os, sys

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

import json

FOLDER_MRTK = '../consumptions/mrtk/'
FOLDER_ENERGYMRTK = '../consumptions/energymrtk/'

def process(folder, isPrint=False):
	"""
	Process 1 experiment only.
	Just change getStatistics() and getListConsumptions() as needed
	"""
	print('folder='+folder)
	if not isPrint:
		sys.stdout = open(os.devnull, 'w')

	durationPlacementList	= getListOfMeasures("{}log_emrtk_1_run.txt".format(folder))  # list of times spent to place a job
	print (durationPlacementList)
	#exit(0)
	mean_			= np.mean(np.array(durationPlacementList))  # the avg, scalar
	min_ 			= np.min(np.array(durationPlacementList))
	max_ 			= np.max(np.array(durationPlacementList))
	var_ 			= np.var(np.array(durationPlacementList))
	all 			= durationPlacementList
	assert len(durationPlacementList) > 0
	
	if not isPrint:
		sys.stdout = sys.__stdout__

	return min_, max_, mean_, var_, all

def getListOfMeasures(filename):
	measures = []  # times list
	with open(filename, "r") as ff:
		for line in ff:
			m = re.search(r"t=(\d+), placement algorithm - (\d+) us", line)
			if m is not None:
                                val = float(m.groups()[1])
                                if val <= 300:  # not an outlier (cout when job cannot be placed)
            				measures.append(val)
	return measures


################################### utility functions

def getFoldersInFolder(folder):
	return os.listdir(folder)

def getFilesInFolder(folder):
	return glob.glob(folder + '*.txt') # att, ~/folder/*.txt non funziona

def getFilesWith(paths, substr):
	res = []
	for p in paths:
		if substr in p:
			res.append(p)
	return res


def addToGraph(xy, label, linestyle = '--'):  # https://matplotlib.org/gallery/lines_bars_and_markers/line_styles_reference.html
	# xy = { time -> voltage }
	x = list(xy.keys())
	y = list(xy.values())
	print('x=%s' % str(x))
	print('y=%s' % str(y))
	plt.plot(x, y, label=label, color='black', linestyle=linestyle)

	plt.xticks(np.arange(min(x), max(x)+1, 200))
	
	plt.xlabel('time (ms)')
	plt.ylabel('W')
	plt.legend()

def packGraph(filename = ''):
	if filename is not '':
		plt.savefig(filename)

	plt.show()


def computeAvgArray(array):
	# returns the average of an array. e.g., array = [0, 8, 4, 6] -> computeAvgArray(array) = 9
	avg = 0.0
	for i in range(0, len(array)):
		avg += array[i]
	avg = avg / len(array)
	return avg

def computeAvgDictionaries(dicts):
	# dictionaries list: dictionaries list, each one of type whatever -> double or int: { 0: 0.2332, 1: 0.3233 }
	# e.g., dicts = [ { 0: 0.4, 1: 2.2 }, { 0: 8.0, 1: 8.2 } ] -> computeAvgDictionaries(dicts) = { 0: 4.2, 1: 5.2 }
	avgDict = {} # whatever -> avg of the doubles or ints

	for i in range(1, len(dicts)):
		if len (dicts[i-1]) != len (dicts[i]):
			print ('Error, dictionaries have different number of entries')
			sys.exit(1)

	print ("You gave " + str(len(dicts)) + " dictionaries and all of them have length " + str(len(dicts[0])))
	for curTime in range(0, len(dicts[0])):  # for each time
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
	avgValueL_aux 	= {}
	avgValueB_aux 	= {}
	maxValueL_aux 	= 0.0
	minValueL_aux 	= 0.0
	maxValueB_aux 	= 0.0
	minValueB_aux 	= 0.0
	sumL_aux 		= 0.0
	sumB_aux 		= 0.0

	maxValueL_aux 	= computeAvgArray(maxValueL)
	minValueL_aux 	= computeAvgArray(minValueL)
	sumL_aux 		= computeAvgArray(sumL)
	maxValueB_aux 	= computeAvgArray(maxValueB)
	minValueB_aux 	= computeAvgArray(minValueB)
	sumB_aux 		= computeAvgArray(sumB)

	avgValueB_aux 	= computeAvgDictionaries(avgValueB)
	avgValueL_aux	= computeAvgDictionaries(avgValueL)

	return avgValueL_aux, maxValueL_aux, minValueL_aux, sumL_aux, avgValueB_aux, maxValueB_aux, minValueB_aux, sumB_aux


################################### main

if __name__ == "__main__":
	listDurations = []
	
	print('found # experiments: ' + str(len(getFoldersInFolder(FOLDER_ENERGYMRTK))))
	for i in range(0, len(getFoldersInFolder(FOLDER_ENERGYMRTK))):
		print("processing experiment " + str(i))
		min, max, mean, var, all = process(FOLDER_ENERGYMRTK + str(i) + '/', False)  # scalar, the avg for the experiment
		listDurations.append({'min': min, 'max': max, 'var': var, 'mean': mean, 'all': all})

	# average duration of the placement algorithm
	avgDuration = 0
	for i in range(0, len(getFoldersInFolder(FOLDER_ENERGYMRTK))):
		avgDuration += listDurations[i]['mean']
	avgDuration = avgDuration / len(listDurations)
	print('Found {} statistics (expected {})'.format(len(listDurations), len(getFoldersInFolder(FOLDER_ENERGYMRTK))))
	assert len(listDurations) == len(getFoldersInFolder(FOLDER_ENERGYMRTK))
	print("Average placement duration over the 10 exps: " + str(avgDuration))

	# overall results
	data = {
		'placement algorithm' : {
			'experiments no' : len(getFoldersInFolder(FOLDER_ENERGYMRTK)),
			'avg duration over the experiments' : avgDuration,
		},
	}
	# add min, max, mean, variance for each experiment
	for i in range(0, len(getFoldersInFolder(FOLDER_ENERGYMRTK)) ): # for each experiment
		data['placement algorithm']['exp no ' + str(i)] = {
			'min' : listDurations[i]['min'],
			'max' : listDurations[i]['max'],
			'mean' : listDurations[i]['mean'],
			'variance' : listDurations[i]['var'],
		}
	if os.path.exists("placement_time_py_output.json"):
		os.remove("placement_time_py_output.json")
	with open('placement_time_py_output.json', 'w') as outfile:
		json.dump(data, outfile, indent=4)
