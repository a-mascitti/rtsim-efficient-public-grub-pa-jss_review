#!/usr/bin/python2

import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import json
import os, sys

def process(folder, isPrint=False):
		"""
		Process 1 util only.
		Just change getStatistics() and getListConsumptions() as needed
		"""
		if not isPrint:
				sys.stdout = open(os.devnull, 'w')

		files = [
				"graph_py_output.json",
		]

		energies = []  # [total energy emrtk, mrtk] for each exp
		for i in range(0, EXPERIMENTS_NO):
			exp_folder = folder + "-" + str(i) + "/"
			for file in files:
				temp_emrtk_big     = 0
				temp_mrtk_big      = 0
				temp_grubpa_big    = 0
				temp_emrtk_little  = 0
				temp_mrtk_little   = 0
				temp_grubpa_little = 0
				filename   = exp_folder + file
				with open(filename, 'r') as f:
					for line in f:
						if "sum energy consumptions big emrtk\"" in line:
							temp_emrtk_big = float(line.replace("\"sum energy consumptions big emrtk\": ","").replace(",",""))
						if "sum energy consumptions little emrtk\"" in line:
							temp_emrtk_little = float(line.replace("\"sum energy consumptions little emrtk\": ","").replace(",",""))
						if "sum energy consumptions big emrtk_bf" in line:
							temp_emrtk_bf_big = float(line.replace("\"sum energy consumptions big emrtk_bf\": ","").replace(",",""))
						if "sum energy consumptions little emrtk_bf" in line:
							temp_emrtk_bf_little = float(line.replace("\"sum energy consumptions little emrtk_bf\": ","").replace(",",""))
						if "sum energy consumptions big emrtk_ff" in line:
							temp_emrtk_ff_big = float(line.replace("\"sum energy consumptions big emrtk_ff\": ","").replace(",",""))
						if "sum energy consumptions little emrtk_ff" in line:
							temp_emrtk_ff_little = float(line.replace("\"sum energy consumptions little emrtk_ff\": ","").replace(",",""))
						if "sum energy consumptions big mrtk" in line:
							temp_mrtk_big = float( line.replace("\"sum energy consumptions big mrtk\": ","").replace(",","") )
						if "sum energy consumptions little mrtk" in line:
							temp_mrtk_little = float( line.replace("\"sum energy consumptions little mrtk\": ","").replace(",","") )
						if "sum energy consumptions big grubpa" in line:
							temp_grubpa_big = float(line.replace("\"sum energy consumptions big grubpa\": ", "").replace(",", ""))
						if "sum energy consumptions little grubpa" in line:
							temp_grubpa_little = float(line.replace("\"sum energy consumptions little grubpa\": ", "").replace(",", ""))
				energies.append( [ temp_emrtk_big, temp_emrtk_little, temp_mrtk_big, temp_mrtk_little, temp_grubpa_big, temp_grubpa_little, temp_emrtk_bf_big, temp_emrtk_bf_little, temp_emrtk_ff_big, temp_emrtk_ff_little ] )

                print str(temp_grubpa_big)

		means 	= []
		vars 	= []
		means.append( np.mean([energies[i][0] for i in range(0, EXPERIMENTS_NO)]) )
		means.append( np.mean([energies[i][1] for i in range(0, EXPERIMENTS_NO)]) )
		means.append( np.mean([energies[i][2] for i in range(0, EXPERIMENTS_NO)]) )
		means.append( np.mean([energies[i][3] for i in range(0, EXPERIMENTS_NO)]) )
		means.append(np.mean([energies[i][4] for i in range(0, EXPERIMENTS_NO)]))
		means.append(np.mean([energies[i][5] for i in range(0, EXPERIMENTS_NO)]))
		means.append(np.mean([energies[i][6] for i in range(0, EXPERIMENTS_NO)]))
		means.append(np.mean([energies[i][7] for i in range(0, EXPERIMENTS_NO)]))
		means.append(np.mean([energies[i][8] for i in range(0, EXPERIMENTS_NO)]))
		means.append(np.mean([energies[i][9] for i in range(0, EXPERIMENTS_NO)]))
		vars.append( np.std([energies[i][0] for i in range(0, EXPERIMENTS_NO)]) )
		vars.append( np.std([energies[i][1] for i in range(0, EXPERIMENTS_NO)]) )
		vars.append( np.std([energies[i][2] for i in range(0, EXPERIMENTS_NO)]) )
		vars.append( np.std([energies[i][3] for i in range(0, EXPERIMENTS_NO)]) )
		vars.append(np.std([energies[i][4] for i in range(0, EXPERIMENTS_NO)]))
		vars.append(np.std([energies[i][5] for i in range(0, EXPERIMENTS_NO)]))
		vars.append(np.std([energies[i][6] for i in range(0, EXPERIMENTS_NO)]))
		vars.append(np.std([energies[i][7] for i in range(0, EXPERIMENTS_NO)]))
		vars.append(np.std([energies[i][8] for i in range(0, EXPERIMENTS_NO)]))
		vars.append(np.std([energies[i][9] for i in range(0, EXPERIMENTS_NO)]))

		if not isPrint:
				sys.stdout = sys.__stdout__

		# means and vars of energies in this order: [ emrtk_big, emrtk_little, mrtk_big, mrtk_little ]
		return means, vars

def printDictionaryToFile(dd, filename):
	if os.path.exists(filename):
		os.remove(filename)

	with open(filename,'w') as file:
	    for k in sorted (dd.keys()):
	        file.write("%s %s\n" % (k, dd[k]))

# ------------------------------------------------ graphs

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
		
		plt.xlabel('time (us)')
		plt.ylabel('W')
		plt.legend(loc='upper right')

def packGraph(filename = '', isShow=False):
		if filename is not '':
				plt.savefig(filename, bbox_inches = "tight")

		if isShow:
				plt.show()



if __name__ == "__main__":
	variances 	= []
	means 		= []
	EXPERIMENTS_NO=int(sys.argv[1])

	#utils = [ 0.25, 0.4, 0.5, 0.65 ]
	utils=[0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7]
        utils8=[u*8 for u in utils]
	for u in utils:
		m, v = process("run-" + str(u), True)
		means.append(m) # m = [ avg temp_emrtk_big, avg temp_emrtk_little, avg temp_mrtk_big, avg temp_mrtk_little ], Wns
		variances.append(v)

	assert len(variances) == len(utils)
	assert len(means) == len(utils)

	plt.xlabel('Total utilization')
	plt.ylabel('Total energy cons. (mJ)')
	plt.grid()
	plotemrtk = plt.errorbar(utils8, [ (means[i][0] + means[i][1]) / 1000000 for i in range(0, len(means)) ], [ (variances[i][0] + variances[i][1]) / 1000000  for i in range(0, len(variances)) ], linestyle='-', marker='*', color='green')
	plotmrtk  = plt.errorbar(utils8, [ (means[i][2] + means[i][3]) / 1000000  for i in range(0, len(means)) ], [ (variances[i][2] + variances[i][3]) / 1000000  for i in range(0, len(variances)) ], linestyle='--', marker='o', color='black')
	plotgrubpa = plt.errorbar(utils8, [(means[i][4] + means[i][5]) / 1000000 for i in range(0, len(means))], [(variances[i][4] + variances[i][5]) / 1000000 for i in range(0, len(variances))], linestyle='-', marker='s', color='black')
	plotemrtk_ff = plt.errorbar(utils8, [ (means[i][8] + means[i][9]) / 1000000 for i in range(0, len(means)) ], [ (variances[i][8] + variances[i][9]) / 1000000  for i in range(0, len(variances)) ], linestyle='-', marker='s', color='blue')
	plotemrtk_bf = plt.errorbar(utils8, [ (means[i][6] + means[i][7]) / 1000000 for i in range(0, len(means)) ], [ (variances[i][6] + variances[i][7]) / 1000000  for i in range(0, len(variances)) ], linestyle='--', marker='.', color='red')
	plt.legend([plotemrtk, plotmrtk, plotgrubpa, plotemrtk_ff, plotemrtk_bf], ['BL-CBS', 'G-EDF', 'GRUB-PA', 'EDF-FF', 'EDF-BF'], loc='lower right')
	plt.xticks(utils8)
	plt.yscale('log')
	packGraph('energies_utilizations.pdf')

	data = {
		"total energy emrtk for utils for bigs " + str(utils) : str([ means[i][0] for i in range(0, len(means)) ]),
		"total energy emrtk for utils for littles " + str(utils) : str([ means[i][1] for i in range(0, len(means)) ]),
		"total energy mrtk for utils for bigs " + str(utils) : str([ means[i][2] for i in range(0, len(means)) ]),
		"total energy mrtk for utils for littles " + str(utils) : str([ means[i][3] for i in range(0, len(means)) ]),
		"total energy grubpa for utils for bigs " + str(utils): str([means[i][4] for i in range(0, len(means))]),
		"total energy grubpa for utils for littles " + str(utils): str([means[i][5] for i in range(0, len(means))]),
		"total energy emrtk_bf for utils for bigs " + str(utils) : str([ means[i][6] for i in range(0, len(means)) ]),
		"total energy emrtk_bf for utils for littles " + str(utils) : str([ means[i][7] for i in range(0, len(means)) ]),
		"total energy emrtk_ff for utils for bigs " + str(utils): str([means[i][8] for i in range(0, len(means))]),
		"total energy emrtk_ff for utils for littles " + str(utils): str([means[i][9] for i in range(0, len(means))]),
	}

	filename = "energies_utilizations.dat"
	print("Data saved into file " + filename)
	printDictionaryToFile(data, filename)
