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
	utilLimitLittle = 0.345328
	ets = []  # all the tasks
	heavyTasks = []
	lightTasks = []
	fileCheckProp = open("check_proposition.dat", 'w+')
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
				# print line
				ets.append(CBServerCallingEMRTKernel(float(split[2]), float(split[3])))

	# sort by tasks by CBS utilization
	ets = sorted(ets, key=lambda cbs: cbs.getUtilization(), reverse=False)  # 3 4 5
	#print(ets)

	fileCheckProp.write('filename=' + filename + '\n')
	fileCheckProp.write('read {} tasks\n'.format(len(ets)))
	assert len(ets) == 24


	# get maxima
	u_B_max = 0.0
	u_L_max = 0.0
	u_max   = 0.0
	u_B_max_2 = 0.0
        u_max_2 = ets[1].getUtilization()
	u_H_max = 0.0
	for e in ets:
		util = e.getUtilization()
		#print (util)
		if (util <= utilLimitLittle):
			lightTasks.append(e)
			if (util > u_L_max):
				u_L_max = util
		else:
			heavyTasks.append(e)
			if (util > u_B_max):
				u_B_max = util
	u_max = u_B_max
	if u_L_max > u_B_max:
		u_max = u_L_max
	lightTasks = sorted(lightTasks, key=lambda cbs: cbs.getUtilization(), reverse=False)
	heavyTasks = sorted(heavyTasks, key=lambda cbs: cbs.getUtilization(), reverse=False)
	u_H_max = 0.0 if len(heavyTasks) == 0 else heavyTasks[0].getUtilization()
	u_B_max_2 = 0.0 if len(heavyTasks) <= 2 else  heavyTasks[1].getUtilization()

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
	fileCheckProp.write("u_max=" + str(u_max) + "\n")
	fileCheckProp.write("n_B_max=" + str(n_B_max) + "\n")
	fileCheckProp.write("n_L_max=" + str(n_L_max) + "\n")
	fileCheckProp.write("# heavyweight tasks=" + str(nHeavyTasks) + "\n")
	fileCheckProp.write("# lightweight tasks=" + str(nLightTasks) + "\n")
	fileCheckProp.write("u_B_left=" + str(u_B_left) + "\n")
	fileCheckProp.write("n_Bres=" + str(n_Bres) + "\n")

	fileCheckProp.write('\n\n\n')
	fileCheckProp.write("prop. 2. ")
	fileCheckProp.write("big=true; " if len(ets) <= n_B * math.floor(1.0 / u_max) else "big=false; ")
	fileCheckProp.write("little=true; " if len(ets) <= n_L * math.floor(utilLimitLittle / u_max) else "little=false; ")
	fileCheckProp.write('\n')

	# prop. 2:
	fileCheckProp.write("prop. 3. ")
	fileCheckProp.write("big=true; " if len(ets) <= 1 + math.floor((1.0 - u_max) / u_max_2) + (n_B - 1) * math.floor(1.0 / u_max_2) else "big=false; ")
	fileCheckProp.write("little=true; " if len(ets) <= 1 + math.floor((utilLimitLittle - u_max) / u_max_2) + (n_L - 1) * math.floor(utilLimitLittle / u_max_2) else "little=false; ")
	fileCheckProp.write('\n')

	# theorem 1:
	fileCheckProp.write("theorem 1. ")
	h = math.floor(nHeavyTasks / float(n_B))
	k = nHeavyTasks % n_B
        cond1 = True
        if u_B_max > 0.0 and u_B_max_2 > 0.0:
            cond1 = ( nHeavyTasks <= n_B * math.floor(1.0 / u_B_max) or nHeavyTasks <= 1 + math.floor((1 - u_B_max) / u_B_max_2) + (n_B - 1) * math.floor(1 / u_B_max_2) )
	cond2 = ( nLightTasks <= n_L * math.floor(utilLimitLittle / u_L_max) + math.floor((1 - h * u_H_max) / u_L_max) * (n_B - k) + math.floor((1 - (h + 1) * u_H_max) / u_L_max) * k )
	fileCheckProp.write("true" if cond1 and cond2 else "false")
	fileCheckProp.write('\n')


	fileCheckProp.write('\n\n\n')
        fileCheckProp.write("heavyweight tasks:\n")
	for t in heavyTasks:
		fileCheckProp.write(t.toString() + '\n')
        fileCheckProp.write('\n')
	fileCheckProp.write("lightweight tasks:\n")
	for t in lightTasks:
		fileCheckProp.write(t.toString() + '\n')


	fileCheckProp.close()
