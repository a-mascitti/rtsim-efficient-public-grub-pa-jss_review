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

FOLDER_MRTK = 'consumptions/mrtk/'
FOLDER_ENERGYMRTK = 'consumptions/energymrtk/'
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
	files_energymrtk = getFilesInFolder(folder)
	print files_energymrtk

	# consider 1 experiment and take the consumptions for each little and then big
	files_energymrtk_littles	= getFilesWith(files_energymrtk, ["power", "LITTLE"])
	files_energymrtk_bigs 		= getFilesWith(files_energymrtk, ["power", "BIG"])

	TICK_LIMIT = 100000

	cons_little = []
	cons_big 	= []
	for f in files_energymrtk_littles:
		cons = getListConsumptions(f)
		cons_little.append(cons)

	for f in files_energymrtk_bigs:
		cons = getListConsumptions(f)
		cons_big.append(cons)


	avgValueL, maxValueL, minValueL, sumValueL = getStatistics(cons_little)
	avgValueB, maxValueB, minValueB, sumValueB = getStatistics(cons_big)

	for time, watt in avgValueL.items():
		#print ("t=" + str(time) + ", avg little: " + str(avgValueL[time]))
		sumL += watt
	for time, watt in avgValueB.items():
		sumB += watt
	#print ('max big: ' + str(maxValueB))
	#print ('min big: ' + str(minValueB))

	if not isPrint:
		sys.stdout = sys.__stdout__


	# sumValueB = {}: tick -> sum of consumptions on big cores 
	return avgValueL, maxValueL, minValueL, sumL, sumValueL, avgValueB, maxValueB, minValueB, sumB, sumValueB

def getListConsumptions(filename):
	global TICK_LIMIT
	cons = {}  # t -> cons
	print filename

	with open(filename, "r") as ff:
		for line in ff:
			m = re.search(r"Cur W=(\d*\.\d+) (\w+) t=(\d+)", line)
			if int(m.groups()[2]) >= TICK_LIMIT:  # limit for memory & computational limits
				print("tick limit {} reached, stopping reading on tick {}".format(TICK_LIMIT, int(m.groups()[2])))
				break
			cons[int(m.groups()[2])] = float(m.groups()[0])
	return cons

def getStatistics(cons):
	# "cons" is an array of 4 dictionaries { t -> cons } with 1000 entries
	avgValue = {} # for each tick, avg of cons.
	sumValue = {} # for each tick, sum of cons. 
	maxValue = 0.0
	minValue = 9999.0

	# checks
	assert len(cons) == 4
	# assert len(cons[0]) >= TICKS_NO  # not necessary if consumptions stored at non-continuous ticks

	for time in range(0,TICK_LIMIT):
		sumValue[time] = getLastYBeforeX(cons[0], time) + getLastYBeforeX(cons[1], time) + getLastYBeforeX(cons[2], time) + getLastYBeforeX(cons[3], time)  # W (in t=time)
		avgValue[time] = sumValue[time] / 4  # W
		if avgValue[time] > maxValue:
			maxValue = avgValue[time]
		if avgValue[time] < minValue:
			minValue = avgValue[time]

	return avgValue, maxValue, minValue, sumValue

def getLastYBeforeX(dd, beforeX):
	# e.g., dd={ 1: 0.12, 2: 0.44, 10: 0.98 }, beforeX=8 => 0.44
	lastY = -1.0

	if beforeX in dd.keys():
		return dd[beforeX]

	for key, value in dd.items():
		if key >= beforeX:
			break
		lastY = float(value)
	return lastY

################################### utility functions

def getFoldersInForlder(folder):
	return os.listdir(folder)

def getFilesInFolder(folder):
	return glob.glob(folder + '*.txt') # att, ~/folder/*.txt non funziona

def getFilesWith(paths, substrs):
	res = []
	for p in paths:
		found = True
		for substr in substrs:
			if substr not in p:
				found = False
		if found:
			res.append(p)
	return res

def updateTicksNo(filename):
	global TICKS_NO, TICK_LIMIT
	stringa = os.popen("head -n 25 {}".format(filename)).read()
	m = re.search("simulation_steps=(\d+)", stringa)
	TICKS_NO = int(m.groups()[0])
	TICK_LIMIT = TICKS_NO
	print("ticks are " + str(TICKS_NO))

def newGraph():
	plt.figure()

def addToGraph(xy, label, linestyle = '--'):  # https://matplotlib.org/gallery/lines_bars_and_markers/line_styles_reference.html
	# xy = { time -> voltage }
	x = list(xy.keys())
	y = list(xy.values())
	#print('x=%s' % str(x))
	#print('y=%s' % str(y))
	plt.plot(x, y, label=label, color='black', linestyle=linestyle)

	print(x)
	print(len(x))
	print(len(x)%10)
	plt.xticks(np.arange(min(x), max(x)+1, len(x) / 10.0), rotation='vertical')
	
	plt.xlabel('time (ms)')
	plt.ylabel('W')
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

	with open(filename,'w') as file:
		for k in sorted (dd.keys()):
			file.write("%s %s\n" % (k, dd[k]))

def printNumpyNdarrayToFile(dd, filename):
	if os.path.exists(filename):
		os.remove(filename)

	s = ""
	for row in dd:
		for col in row:
			s += str(col) + " "
		s += '\n'

	with open(filename,'w') as file:
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
	names = ['id','data']
	formats = ['int64','f8']
	dtype = dict(names = names, formats=formats)
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
	sumDict = {} # whatever -> avg of the doubles or ints

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
	sumB_eachTick_EMRTK = {}  # tick -> sum of consumptions of big cores
	sumL_eachTick_EMRTK = {}
	sumB_eachTick_MRTK  = {}
	sumL_eachTick_MRTK  = {}
	
	print('graph.py, found # experiments: ' + str(len(getFoldersInForlder(FOLDER_ENERGYMRTK))))
	for i in range(0, len(getFoldersInForlder(FOLDER_ENERGYMRTK))):
		print("processing experiment " + str(i))
		updateTicksNo(FOLDER_ENERGYMRTK + str(i) + '/log_emrtk_1_run.txt')
		avgValueL_EMRTK[i], maxValueL_EMRTK[i], minValueL_EMRTK[i], sumL_EMRTK[i], sumL_eachTick_EMRTK[i], avgValueB_EMRTK[i], maxValueB_EMRTK[i], minValueB_EMRTK[i], sumB_EMRTK[i], sumB_eachTick_EMRTK[i] = process(FOLDER_ENERGYMRTK + str(i) + '/')
		avgValueL_MRTK[i], maxValueL_MRTK[i], minValueL_MRTK[i], sumL_MRTK[i], sumL_eachTick_MRTK[i], avgValueB_MRTK[i], maxValueB_MRTK[i], minValueB_MRTK[i], sumB_MRTK[i], sumB_eachTick_MRTK[i] = process(FOLDER_MRTK + str(i) + '/')

	avgValueL_EMRTK, maxValueL_EMRTK, minValueL_EMRTK, sumL_EMRTK, avgValueB_EMRTK, maxValueB_EMRTK, minValueB_EMRTK, sumB_EMRTK = computeAvg(avgValueL_EMRTK, maxValueL_EMRTK, minValueL_EMRTK, sumL_EMRTK, avgValueB_EMRTK, maxValueB_EMRTK, minValueB_EMRTK, sumB_EMRTK)
	avgValueL_MRTK,  maxValueL_MRTK,  minValueL_MRTK,  sumL_MRTK,  avgValueB_MRTK,  maxValueB_MRTK,  minValueB_MRTK,  sumB_MRTK  = computeAvg(avgValueL_MRTK,  maxValueL_MRTK,  minValueL_MRTK,  sumL_MRTK,  avgValueB_MRTK,  maxValueB_MRTK,  minValueB_MRTK,  sumB_MRTK)


	# avgSumB_eachTick_EMRTK = computeSumDictionaries(sumB_eachTick_EMRTK)
	# avgSumL_eachTick_EMRTK = computeSumDictionaries(sumL_eachTick_EMRTK)
	# avgSumB_eachTick_MRTK = computeSumDictionaries(sumB_eachTick_MRTK)
	# avgSumL_eachTick_MRTK = computeSumDictionaries(sumL_eachTick_MRTK)

	# print ("checking processed results...")
	# print (avgValueB_EMRTK[1] 	== 1.7353896)
	# print (avgValueB_EMRTK[400] == 1.0481579)
	# print (avgValueB_EMRTK[999] == 0.4668979)
	# print ('checked')

	# and now big consumptions
	print('Now making graphs:')

	# addToGraph(sumB_eachTick_EMRTK[0], 'big, paper alg.', '-')
	# addToGraph(sumB_eachTick_MRTK[0], 'big, GEDF', '--')
	# packGraph('results_big_exp0.pdf')
	
	# newGraph()
	# addToGraph(sumL_eachTick_EMRTK[0], 'LITTLE, paper alg.', '-')
	# addToGraph(sumL_eachTick_MRTK[0], 'LITTLE, GEDF.', '--')
	# packGraph('results_LITTLE_exp0.pdf')
	
	print("exporting data for making graphs (y=W)")
	printDictionaryToFile(sumB_eachTick_EMRTK[0], 'sumB_eachTick_EMRTK_exp0.dat')  # y=W
	printDictionaryToFile(sumB_eachTick_MRTK[0],  'sumB_eachTick_MRTK_exp0.dat')
	
	printDictionaryToFile(sumL_eachTick_EMRTK[0], 'sumL_eachTick_EMRTK_exp0.dat')  # y=W
	printDictionaryToFile(sumL_eachTick_MRTK[0],  'sumL_eachTick_MRTK_exp0.dat')

	os.system("gnuplot graph.gp")
	print("plot PDF generated")

	
	print("exporting data for making graphs (y=energy)")
	lastW = -1
	energy = 0.0
	dd = sumB_eachTick_EMRTK[0]
	dd = list(dictToMatrix(dd))  # [ (0, 0.123), (1000, 0.33), (1200, 0.123) ] : list
	energy_time = list()
	energy_time.append( (0, dd[0][1]) )  # working with lists/ndarray is much much much faster than working with dicts
	for i in range(1, len(dd) - 1 ):
		lastW = dd[i][1]
		energy += lastW * abs(dd[i+1][0] - dd[i][0])
		energy_time.append( (dd[i+1][0], energy) )
	energyB_EMRTK = energy
	energyB_EMRTK_time = np.asarray(energy_time)

	lastW = -1
	energy = 0.0
	dd = sumL_eachTick_EMRTK[0]
	dd = list(dictToMatrix(dd))  # [ (0, 0.123), (1000, 0.33), (1200, 0.123) ] : list
	energy_time = list()
	energy_time.append( (0, dd[0][1]) )  # working with lists/ndarray is much much much faster than working with dicts
	for i in range(1, len(dd) - 1 ):
		lastW = dd[i][1]
		energy += lastW * abs(dd[i+1][0] - dd[i][0])
		energy_time.append( (dd[i+1][0], energy) )
	energyL_EMRTK = energy
	energyL_EMRTK_time = np.asarray(energy_time)

	lastW = -1
	energy = 0.0
	dd = sumB_eachTick_MRTK[0]
	dd = list(dictToMatrix(dd))  # [ (0, 0.123), (1000, 0.33), (1200, 0.123) ] : list
	energy_time = list()
	energy_time.append( (0, dd[0][1]) )  # working with lists/ndarray is much much much faster than working with dicts
	for i in range(1, len(dd) - 1 ):
		lastW = dd[i][1]
		energy += lastW * abs(dd[i+1][0] - dd[i][0])
		energy_time.append( (dd[i+1][0], energy) )
	energyB_MRTK = energy
	energyB_MRTK_time = np.asarray(energy_time)

	lastW = -1
	energy = 0.0
	dd = sumL_eachTick_MRTK[0]
	dd = list(dictToMatrix(dd))  # [ (0, 0.123), (1000, 0.33), (1200, 0.123) ] : list
	energy_time = list()
	energy_time.append( (0, dd[0][1]) )  # working with lists/ndarray is much much much faster than working with dicts
	for i in range(1, len(dd) - 1 ):
		lastW = dd[i][1]
		energy += lastW * abs(dd[i+1][0] - dd[i][0])
		energy_time.append( (dd[i+1][0], energy) )
	energyL_MRTK = energy
	energyL_MRTK_time = np.asarray(energy_time)

	print("Dumping energies over time to files")
	printNumpyNdarrayToFile(energyB_EMRTK_time, "energyB_EMRTK_time.dat")  # this is very very very fast
	printNumpyNdarrayToFile(energyL_EMRTK_time, "energyL_EMRTK_time.dat")
	printNumpyNdarrayToFile(energyB_MRTK_time, "energyB_MRTK_time.dat")
	printNumpyNdarrayToFile(energyL_MRTK_time, "energyL_MRTK_time.dat")

	os.system("gnuplot energy.gp")
	print("plot PDF generated")


	print ('big energy consumption. emrtk VS mrtk: ' + str(energyB_EMRTK) + " VS " + str(energyB_MRTK))

	print ('paper sum: ' + str(energyB_EMRTK + energyL_EMRTK))
	print ('EDF sum: ' + str(energyB_MRTK + energyL_MRTK))

	data = {
		'energy consumptions' : {
				'sum energy consumptions big emrtk' : energyB_EMRTK,
				'sum energy consumptions big mrtk'  : energyB_MRTK,
				'sum energy consumptions little emrtk' : energyL_EMRTK,
				'sum energy consumptions little mrtk'  : energyL_MRTK,
				
				'paper alg big + little': energyL_EMRTK + energyB_EMRTK,
				'EDF big + little' : energyL_MRTK + energyB_MRTK,

				'avg energy consumption big emrtk' : energyB_EMRTK / TICK_LIMIT,
				'avg energy consumption little emrtk' : energyL_EMRTK / TICK_LIMIT,
				'avg energy consumption big mrtk' : energyB_MRTK / TICK_LIMIT,
				'avg energy consumption little mrtk' : energyL_MRTK / TICK_LIMIT
		},
	}
	if os.path.exists("graph_py_output.json"):
		os.remove("graph_py_output.json")
	with open('graph_py_output.json', 'w') as outfile:
		json.dump(data, outfile, indent=4)
