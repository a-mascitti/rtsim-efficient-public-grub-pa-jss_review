#!/usr/bin/python2

# input: 	cartelle energymrtk/0/ ... energymrtk/9 e mrtk/0 ... mrtk/9 con all'interno i dump dei consumi
# 			per ogni core da t=0 a t=1000.
# output:	metriche di consumo per energymrtk e per mrtk: media per ogni t (=per righe) e somma dei consumi 
#			little e big, oltre a min e max per isola

import glob, re, os, sys

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import json

FOLDER_MRTK = './consumptions/mrtk/'
FOLDER_ENERGYMRTK = './consumptions/energymrtk/'
TICKS_NO = 6000  # this parameter can be deleted with some code modifications

def process(folder, isPrint=False):
	"""
	Process 1 experiment only.
	Just change getStatistics() and getListFromPattern() as needed
	"""
	if not isPrint:
		sys.stdout = open(os.devnull, 'w')
	
	filename = folder + 'freqBIG.txt'
	temp_freq_big = getListFromPattern(filename)
	filename = folder + 'freqLITTLE.txt'
	temp_freq_little = getListFromPattern(filename)

	if not isPrint:
		sys.stdout = sys.__stdout__

	return temp_freq_big, temp_freq_little

def getListFromPattern(filename):
	temp_freq = {}  # t -> freq
	with open(filename, "r") as ff:
		for line in ff:
			m = re.search(r"(\d*) (\w+) (\d*)", line)
			temp_freq[int(m.groups()[0])] = int(m.groups()[2])
	return temp_freq

def getStatistics(cons):
	return
	# "cons" is an array of 4 dictionaries { t -> cons } with 1000 entries
	avgValue = {}
	maxValue = 0.0
	minValue = 9999.0

	# checks
	assert len(cons) == 4
	assert len(cons[0]) >= TICKS_NO

	for time in range(0,TICKS_NO):
		avgValue[time] = ( float(cons[0][str(time)]) + float(cons[1][str(time)]) + float(cons[2][str(time)]) + float(cons[3][str(time)]) ) / 4
		if avgValue[time] > maxValue:
			maxValue = avgValue[time]
		if avgValue[time] < minValue:
			minValue = avgValue[time]

	return avgValue, maxValue, minValue

################################### utility functions

def getFoldersInForlder(folder):
	return os.listdir(folder)

def getFilesInFolder(folder):
	return glob.glob(folder + '*.txt') # att, ~/folder/*.txt non funziona

def getFilesWith(paths, substr):
	res = []
	for p in paths:
		if substr in p:
			res.append(p)
	return res

def updateTicksNo(filename):
	global TICKS_NO
	stringa = os.popen("head -n 25 {}".format(filename)).read()
	m = re.search("simulation_steps=(\d+)", stringa)
	TICKS_NO = int(m.groups()[0])

def newGraph():
	plt.figure()

def addToGraph(xy, label, linestyle = '--', xlabel='time (ms)', ylabel='W'):  # https://matplotlib.org/gallery/lines_bars_and_markers/line_styles_reference.html
	# xy = { time -> voltage }
	x = list(xy.keys())
	y = list(xy.values())
	#print('x=%s' % str(x))
	#print('y=%s' % str(y))
	plt.plot(x, y, label=label, color='black', linestyle=linestyle)

	plt.xticks(np.arange(min(x), max(x)+1, len(x) / 100.0), rotation='vertical')
	
	plt.xlabel(xlabel)
	plt.ylabel(ylabel)
	plt.legend(loc='upper right')

def packGraph(filename = '', isShow=False):
	if filename is not '':
		plt.savefig(filename, bbox_inches = "tight")

	if isShow:
		plt.show()

def completeDictionaryWithMissingTicks(dd, untilKey=-1):
	# adds the missing ticks to a dictionary. The corresponding y will be the last found one
	# e.g., dd={ 0: 5, 3: 7, 5: 1 } => { 0: 5, 1: 5, 2: 5, 3: 7, 4: 7, 5: 1 }
	assert len(dd) > 2

	res = {}
	lastY = -1.0
	for i in xrange(0, len(dd.keys()) ):
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
	# e.g., dd={ 0: 5, 2: 5, 3: 7, 4: 7, 5: 1 }, keysNo=1 => { 0: 5 }

	print("len before trim {}".format(len(dd)))
	to_remove = dd.keys()[keysNo:]
	for key in to_remove:
		del dd[key]
	print("len after trim {}".format(len(dd)))
	return dd

def printDictionaryToFile(dd, filename):
	if os.path.exists(filename):
		os.remove(filename)

	with open(filename,'w') as file:
	    for k in sorted (dd.keys()):
	        file.write("%s %s\n" % (k, dd[k]))

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
	return
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
	time_freq_bigs = {}  # list of dictionaries of type time -> freq for big island
	time_freq_littles = {}
	onlyOne = True

	if len(sys.argv) > 1 and sys.argv[1] == '--only-one':  # graph of freq of only exp 0
		onlyOne = True
	
	print('freq.py, found # experiments: ' + str(len(getFoldersInForlder(FOLDER_ENERGYMRTK))))
	for i in range(0, len(getFoldersInForlder(FOLDER_ENERGYMRTK))):
		print("processing experiment " + str(i))
		updateTicksNo(FOLDER_ENERGYMRTK + '/' + str(i) + '/log_emrtk_1_run.txt')
		time_freq_bigs[i], time_freq_littles[i] = process(FOLDER_ENERGYMRTK + '/' + str(i) + '/', True)
		if onlyOne:
			break

	if onlyOne:
		avg_time_freq_bigs = time_freq_bigs[0]
		avg_time_freq_littles = time_freq_littles[0]
	else:
		avg_time_freq_bigs = computeAvgDictionaries(time_freq_bigs)
		avg_time_freq_littles = computeAvgDictionaries(time_freq_littles)

	# and now big consumptions
	print("")
	print('Now making freq graph:')
	assert len(time_freq_bigs) <= 100000
	assert len(time_freq_littles) <= 100000
	
	print("exporting data for making graphs")
	printDictionaryToFile(avg_time_freq_bigs, 'freqBIG.dat')
	printDictionaryToFile(avg_time_freq_littles, 'freqLITTLE.dat')
	
	os.system("gnuplot freq.gp")
	print("plot PDF generated")

	if onlyOne:
		# packGraph('results_freq_over_time_exp0.pdf')

		data = {
			'avg freq over the experiment 0 (only 1 exp considered)' : {
					'avg freq big: ' : str(np.mean(np.array(avg_time_freq_bigs.values()))),
					'avg freq LITTLE: ' : str(np.mean(np.array(avg_time_freq_littles.values()))),
			},
			'max freq big' : np.max(np.array(avg_time_freq_bigs.values())),
			'max freq LITTLE' : np.max(np.array(avg_time_freq_littles.values())),
			'min freq big' : np.min(np.array(avg_time_freq_bigs.values())),
			'min freq LITTLE' : np.min(np.array(avg_time_freq_littles.values())),
		}
		print(data)
		if os.path.exists("freq_py_output_exp0.json"):
			os.remove("freq_py_output_exp0.json")
		with open('freq_py_output_exp0.json', 'w') as outfile:
			json.dump(data, outfile, indent=4)
	else:
		# addToGraph(avg_time_freq_bigs, 'paper alg. big', '-', 'time (ms)', 'freq. (MHz)')
		# addToGraph(avg_time_freq_littles, 'paper alg. LITTLE', '--', 'time (ms)', 'freq. (MHz)')
		# packGraph('results_avg_freq_over_time.pdf')

		avg_time_freq_bigs 		= np.mean(np.array(avg_time_freq_bigs.values()))
		avg_time_freq_littles 	= np.mean(np.array(avg_time_freq_littles.values()))

		print ('avg freq big: ' + str(avg_time_freq_bigs))
		print ('avg freq LITTLE: ' + str(avg_time_freq_littles))

		data = {
			'avg freq over the 10 experiments' : {
					'avg freq big: ' : str(avg_time_freq_bigs),
					'avg freq LITTLE: ' : str(avg_time_freq_littles),
			},
		}
		if os.path.exists("freq_py_output.json"):
			os.remove("freq_py_output.json")
		with open('freq_py_output.json', 'w') as outfile:
			json.dump(data, outfile, indent=4)
