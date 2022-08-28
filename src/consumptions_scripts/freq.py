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
FOLDER_GRUBPA = './consumptions/grub_pa/'
FOLDER_ENERGYMRTK_BF = './consumptions/energymrtk_bf/'
FOLDER_ENERGYMRTK_FF = './consumptions/energymrtk_ff/'
TICKS_NO = 6000  # this parameter can be deleted with some code modifications

def ASSERT_ROUTINE(expr, msg=''):
	if not expr:
		sys.stderr.write("Error in " + os.path.realpath(__file__) + "; " + msg)
	assert(expr)

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

def getHowMuchTimePerFreq(folder):
		# how much time has each freq been kept?
		kept_l = {}
		kept_b = {}

		for u in xrange(0,2001,100):
			kept_l[str(u)] = 0
			kept_b[str(u)] = 0

		with open(folder + "freqBIG.txt", 'r') as f:
			content = f.readlines()
			for i in xrange(0,len(content) - 1):
				line_i = content[i].replace(' big','').replace(' little','').split(' ')  # 12345 200MHz
				line_iplus1 = content[i+1].replace(' big','').replace(' little','').split(' ')
				kept_b[line_i[1].strip()] = kept_b[line_i[1].strip()] + int(line_iplus1[0]) - int(line_i[0])

		with open(folder + "freqLITTLE.txt", 'r') as f:
			content = f.readlines()
			for i in xrange(0,len(content) - 1):
				line_i = content[i].replace(' big','').replace(' LITTLE','').split(' ')
				line_iplus1 = content[i+1].replace(' big','').replace(' LITTLE','').split(' ')
				kept_l[line_i[1].strip()] = kept_l[line_i[1].strip()] + int(line_iplus1[0]) - int(line_i[0])

		return kept_b, kept_l


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
	stringa = os.popen("head -n 325 {}".format(filename)).read()
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

def sortDictionaryByKey(dd):
	return sorted(dd.iteritems(), key=lambda key_value: key_value[0])  # ( (key, val), ... )

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
	time_freq_big_emrtk = {}  # list of dictionaries of type time -> freq for big island
	time_freq_little_emrtk = {}
	time_freq_big_emrtk_bf = {}
	time_freq_little_emrtk_bf = {}
	time_freq_big_emrtk_ff = {}
	time_freq_little_emrtk_ff = {}
	time_freq_big_grubpa = {}
	time_freq_little_grubpa = {}
	kept_freq_big_emrtk = {}
	kept_freq_little_emrtk = {}  # 200 for 10000 tick, 300 for 150 tick, etc.
	kept_freq_big_emrtk_bf = {}
	kept_freq_little_emrtk_bf = {}  # 200 for 10000 tick, 300 for 150 tick, etc.
	kept_freq_big_emrtk_ff = {}
	kept_freq_little_emrtk_ff = {}  # 200 for 10000 tick, 300 for 150 tick, etc.
	kept_freq_big_grubpa = {}
	kept_freq_little_grubpa = {}
	avg_time_freq_big_emrtk = 0
	avg_time_freq_little_emrtk = 0
	avg_time_freq_big_emrtk_bf = 0
	avg_time_freq_little_emrtk_bf = 0
	avg_time_freq_big_emrtk_ff = 0
	avg_time_freq_little_emrtk_ff = 0
	avg_time_freq_big_grubpa = 0
	avg_time_freq_little_grubpa = 0

	print('freq.py, found # experiments: ' + str(len(getFoldersInForlder(FOLDER_ENERGYMRTK))))
	updateTicksNo(FOLDER_ENERGYMRTK + '/0/log_emrtk_1_run.txt')
	time_freq_big_emrtk, time_freq_little_emrtk  = process(FOLDER_ENERGYMRTK + '/0/', True)
	time_freq_big_grubpa, time_freq_little_grubpa = process(FOLDER_GRUBPA + '/0/', True)
	time_freq_big_emrtk_bf, time_freq_little_emrtk_bf  = process(FOLDER_ENERGYMRTK_BF + '/0/', True)
	time_freq_big_emrtk_ff, time_freq_little_emrtk_ff  = process(FOLDER_ENERGYMRTK_FF + '/0/', True)

	kept_freq_big_emrtk, kept_freq_little_emrtk = getHowMuchTimePerFreq(FOLDER_ENERGYMRTK + '/0/')
	kept_freq_big_grubpa, kept_freq_little_grubpa = getHowMuchTimePerFreq(FOLDER_GRUBPA + '/0/')
	kept_freq_big_emrtk_bf, kept_freq_little_emrtk_bf = getHowMuchTimePerFreq(FOLDER_ENERGYMRTK_BF + '/0/')
	kept_freq_big_emrtk_ff, kept_freq_little_emrtk_ff = getHowMuchTimePerFreq(FOLDER_ENERGYMRTK_FF + '/0/')

	for k,v in kept_freq_big_emrtk.items():
		avg_time_freq_big_emrtk += int(k) * v
	avg_time_freq_big_emrtk /= float(TICKS_NO)
	for k,v in kept_freq_little_emrtk.items():
		avg_time_freq_little_emrtk += int(k) * v
	avg_time_freq_little_emrtk /= float(TICKS_NO)
	
	for k, v in kept_freq_big_emrtk_bf.items():
		avg_time_freq_big_emrtk_bf += int(k) * v
	avg_time_freq_big_emrtk_bf /= float(TICKS_NO)
	for k, v in kept_freq_little_emrtk_bf.items():
		avg_time_freq_little_emrtk_bf += int(k) * v
	avg_time_freq_little_emrtk_bf /= float(TICKS_NO)
	
	for k, v in kept_freq_big_emrtk_ff.items():
		avg_time_freq_big_emrtk_ff += int(k) * v
	avg_time_freq_big_emrtk_ff /= float(TICKS_NO)
	for k, v in kept_freq_little_emrtk_ff.items():
		avg_time_freq_little_emrtk_ff += int(k) * v
	avg_time_freq_little_emrtk_ff /= float(TICKS_NO)

	for k,v in kept_freq_big_grubpa.items():
		avg_time_freq_big_grubpa += int(k) * v
	avg_time_freq_big_grubpa /= float(TICKS_NO)
	for k,v in kept_freq_little_grubpa.items():
		avg_time_freq_little_grubpa += int(k) * v
	avg_time_freq_little_grubpa /= float(TICKS_NO)
		
	assert len(time_freq_big_emrtk) <= TICKS_NO
	assert len(time_freq_little_emrtk) <= TICKS_NO
	
	assert len(time_freq_big_emrtk_bf) <= TICKS_NO
	assert len(time_freq_little_emrtk_bf) <= TICKS_NO
	
	assert len(time_freq_big_emrtk_ff) <= TICKS_NO
	assert len(time_freq_little_emrtk_ff) <= TICKS_NO

	assert len(time_freq_big_grubpa) <= TICKS_NO
	assert len(time_freq_little_grubpa) <= TICKS_NO
	
	print("exporting data for making graphs")
	printDictionaryToFile(time_freq_big_emrtk, 'freqBIG.dat')
	printDictionaryToFile(time_freq_little_emrtk, 'freqLITTLE.dat')
	
	printDictionaryToFile(time_freq_big_emrtk_bf, 'freqBIG_bf.dat')
	printDictionaryToFile(time_freq_little_emrtk_bf, 'freqLITTLE_bf.dat')
	
	printDictionaryToFile(time_freq_big_emrtk_ff, 'freqBIG_ff.dat')
	printDictionaryToFile(time_freq_little_emrtk_ff, 'freqLITTLE_ff.dat')
	
	printDictionaryToFile(time_freq_big_grubpa, 'freqBIG_grubpa.dat')
	printDictionaryToFile(time_freq_little_grubpa, 'freqLITTLE_grubpa.dat')

	
	total_time = 0
	with open(FOLDER_ENERGYMRTK + '/0/freq_histog.dat', 'w') as f:
		for freq in xrange(0,2001,100):
			f.write(str(freq) + ' ' + str(kept_freq_big_emrtk[str(freq)]) + ' ' + str(kept_freq_little_emrtk[str(freq)]) + '\n')
			total_time += kept_freq_big_emrtk[str(freq)] + kept_freq_little_emrtk[str(freq)]
	ASSERT_ROUTINE(total_time == TICKS_NO * 2, 'wf')

	total_time = 0
	with open(FOLDER_ENERGYMRTK_BF + '/0/freq_histog.dat', 'w') as f:
		for freq in xrange(0, 2001, 100):
			f.write(str(freq) + ' ' + str(kept_freq_big_emrtk_bf[str(freq)]) + ' ' + str(kept_freq_little_emrtk_bf[str(freq)]) + '\n')
			total_time += kept_freq_big_emrtk_bf[str(freq)] + kept_freq_little_emrtk_bf[str(freq)]
	ASSERT_ROUTINE(total_time == TICKS_NO * 2,'bf')

	total_time = 0
	with open(FOLDER_ENERGYMRTK_FF + '/0/freq_histog.dat', 'w') as f:
		for freq in xrange(0, 2001, 100):
			f.write(str(freq) + ' ' + str(kept_freq_big_emrtk_ff[str(freq)]) + ' ' + str(kept_freq_little_emrtk_ff[str(freq)]) + '\n')
			total_time += kept_freq_big_emrtk_ff[str(freq)] + kept_freq_little_emrtk_ff[str(freq)]
	ASSERT_ROUTINE(total_time == TICKS_NO * 2, 'ff')

	total_time = 0
	with open(FOLDER_GRUBPA + '/0/freq_histog.dat', 'w') as f:
		for freq in xrange(0,2001,100):
			f.write(str(freq) + ' ' + str(kept_freq_big_grubpa[str(freq)]) + ' ' + str(kept_freq_little_grubpa[str(freq)]) + '\n')
			total_time += kept_freq_big_grubpa[str(freq)] + kept_freq_little_grubpa[str(freq)]
	ASSERT_ROUTINE(total_time == TICKS_NO * 2, 'grubpa')

	os.system("gnuplot freq.gp")
	print("plot PDF generated")

	data = {
		'avg freq over the experiment 0 (only 1 exp considered)' : {
				'avg freq big emrtk' : str(avg_time_freq_big_emrtk),
				'avg freq LITTLE emrtk' : str(avg_time_freq_little_emrtk),
				
				'avg freq big emrtk_bf' : str(avg_time_freq_big_emrtk_bf),
				'avg freq LITTLE emrtk_bf' : str(avg_time_freq_little_emrtk_bf),
				
				'avg freq big emrtk_ff': str(avg_time_freq_big_emrtk_ff),
				'avg freq LITTLE emrtk_ff': str(avg_time_freq_little_emrtk_ff),
				
				'avg freq big grubpa': str(avg_time_freq_big_grubpa),
				'avg freq LITTLE grubpa': str(avg_time_freq_little_grubpa),
		},
		'max freq big emrtk' : np.max(np.array(time_freq_big_emrtk.values())),
		'max freq LITTLE emrtk' : np.max(np.array(time_freq_little_emrtk.values())),
		'min freq big emrtk' : np.min(np.array(time_freq_big_emrtk.values())),
		'min freq LITTLE emrtk' : np.min(np.array(time_freq_little_emrtk.values())),

		'max freq big emrtk_bf': np.max(np.array(time_freq_big_emrtk_bf.values())),
		'max freq LITTLE emrtk_bf': np.max(np.array(time_freq_little_emrtk_bf.values())),
		'min freq big emrtk_bf': np.min(np.array(time_freq_big_emrtk_bf.values())),
		'min freq LITTLE emrtk_bf': np.min(np.array(time_freq_little_emrtk_bf.values())),

	        'max freq big emrtk_ff': np.max(np.array(time_freq_big_emrtk_ff.values())),
		'max freq LITTLE emrtk_ff': np.max(np.array(time_freq_little_emrtk_ff.values())),
		'min freq big emrtk_ff': np.min(np.array(time_freq_big_emrtk_ff.values())),
		'min freq LITTLE emrtk_ff': np.min(np.array(time_freq_little_emrtk_ff.values())),

		'max freq big grubpa': np.max(np.array(time_freq_big_grubpa.values())),
		'max freq LITTLE grubpa': np.max(np.array(time_freq_little_grubpa.values())),
		'min freq big grubpa': np.min(np.array(time_freq_big_grubpa.values())),
		'min freq LITTLE grubpa': np.min(np.array(time_freq_little_grubpa.values())),
	}

	print(data)
	if os.path.exists("freq_py_output_exp0.json"):
		os.remove("freq_py_output_exp0.json")
	with open('freq_py_output_exp0.json', 'w') as outfile:
		json.dump(data, outfile, indent=4)
	