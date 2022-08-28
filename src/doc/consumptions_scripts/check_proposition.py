#!/usr/bin/python2

import numpy as np
import pandas as pd
import json
import os, sys, math

class CBServerCallingEMRTKernel:
	# _wcet
	# _period

	def __init__(self, wcet, period):
		self._wcet = wcet
		self._period = period

	def getUtilization(self):
		util = self._wcet / self._period
		return util

	def toString(self):
		return str(self._wcet) + ' ' + str(self._period) + ' ' + str(self.getUtilization())

if __name__ == "__main__":
	res = []  # of bool
	msg = ""
	n_B = 4
	n_L = 4
	utilLimitLittle = 0.3325
	ets = []  # all the tasks
	heavyTasks = []
	lightTasks = []
	fileCheckProp = open("checkAgainstPaperPropositions.txt", 'w+')
	fileCheckProp.write("util limit little: " + str(utilLimitLittle) + '\n')

	config = 'taskset_generator/saved.conf'
	filename = ""
	with open(config, 'r') as f:
		for line in f:
			filename = line

	with open(filename, 'r') as f:
		for line in f:
			split = line.split(' ')
			if len(split) == 4:
				print line
				ets.append(CBServerCallingEMRTKernel(float(split[2]), float(split[3])))

	fileCheckProp.write('filename=' + filename + '\n')
	fileCheckProp.write('read {} tasks\n'.format(len(ets)))
	assert len(ets) == 24


	# prop. 1: schedulable if number of heavy tasks <= n_B * n_B^max
	u_B_max = 0
	u_L_max = 0
	for e in ets:
		util = e.getUtilization()
		print (util)
		if (util <= utilLimitLittle):
			lightTasks.append(e)
			if (util > u_L_max):
				u_L_max = util
		else:
			heavyTasks.append(e)
			if (util > u_B_max):
				u_B_max = util

	# get n_B^max and n_L^max
	n_B_max = 0
	n_L_max = 0
	for e in heavyTasks:
		if (e.getUtilization() == u_B_max):
			n_B_max += 1
	for e in lightTasks:
		if (e.getUtilization() == u_L_max):
			n_L_max += 1

	nHeavyTasks = len(heavyTasks)
	nLightTasks = len(ets) - nHeavyTasks
	u_B_left = 1.0 - n_B_max * u_B_max
	n_Bres = math.floor(u_B_left / u_L_max)

	fileCheckProp.write("u_B_max=" + str(u_B_max) + "\n")
	fileCheckProp.write("u_L_max=" + str(u_L_max) + "\n")
	fileCheckProp.write("n_B_max=" + str(n_B_max) + "\n")
	fileCheckProp.write("n_L_max=" + str(n_L_max) + "\n")
	fileCheckProp.write("# heavyweight tasks=" + str(nHeavyTasks) + "\n")
	fileCheckProp.write("# lightweight tasks=" + str(nLightTasks) + "\n")
	fileCheckProp.write("u_B_left=" + str(u_B_left) + "\n")
	fileCheckProp.write("n_Bres=" + str(n_Bres) + "\n")

	fileCheckProp.write('\n\n\n')
	fileCheckProp.write("prop. 1: schedulable if number of heavy tasks <= n_B * n_B^max. Result: ")
	res.append(nHeavyTasks <= n_B * n_B_max)
	if res[0]:
		fileCheckProp.write("true" + "\n")
	else:
		fileCheckProp.write("false" + "\n")

	# prop. 2: schedulable if number of light tasks <= n_L * n_L^max + n_B * n_Bres^max
	fileCheckProp.write("prop. 2: schedulable if number of light tasks <= n_L * n_L^max + n_B * n_Bres^max. Result: ")
	res.append(nLightTasks <= n_L * n_L_max + n_B * n_Bres)
	if res[1]:
		fileCheckProp.write("true" + "\n")
	else:
		fileCheckProp.write("false" + "\n")


	fileCheckProp.write('\n\n\n')
	fileCheckProp.write("heavyweight tasks:")
	for t in heavyTasks:
		fileCheckProp.write(t.toString() + '\n')
	fileCheckProp.write("lightweight tasks:")
	for t in lightTasks:
		fileCheckProp.write(t.toString() + '\n')


	fileCheckProp.close()
