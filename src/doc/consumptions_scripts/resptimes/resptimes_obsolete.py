#!/usr/bin/python2

import sys, os, re
import pandas as pd
import numpy as np

FOLDER_MRTK 	 	= '../mrtk/'
FOLDER_ENERGYMRTK 	= '../energymrtk/'
TICKS_NO = 6000  # this parameter can be deleted with some code modifications

def computeAverageTableByColumns(tables):
	"""
	input: 	tables list containing whatever (not string)
	output: this function takes a list of homogeneous tables (same # entries and columns).
			then, it makes the average of corresponding columns

			Example: 
					data1 = pd.DataFrame({
						'name': [1,2,3],
						'age': [9.8, 3.2, 1.0]
					})

					data2 = pd.DataFrame({
						'name': [1,2,3],
						'age': [8.2, 1.6, 1.0]
					})

					print (data1)
					print (data2)

					joined = [data1, data2]

					avg = computeAverageTableByColumns(joined)
					avg will be:
					   age  name
					0  9.0   1.0
					1  2.4   2.0
					2  1.0   3.0
	"""
	avgTable = pd.DataFrame()

	print '-----'
	rows = len(tables[0])
	for i in range(1, len(tables)):  # take ticks of the exp that lastest the least
			if len(tables[i]) < rows:
					rows = len(tables[i])
	columns = len(tables[0].columns)
	columns_names = tables[0].columns
	tables_no = len(tables)
	print 'rows=' + str(rows)
	print 'cols=' + str(columns)
	print 'tables_no=' + str(tables_no)
	print 'columns_names=' + str(columns_names)
	for c in range(0, columns):
		avgCol = []
		column_name = columns_names[c]
		print ('Elaborating column: ' + column_name)
		for r in range(0, rows):
			avg = 0.0
			for t in range(0, tables_no):
				# print 'r=' + str(r) + ' c=' + str(c) + ' t=' + str(t)
				avg += tables[t][column_name].iloc[r]
			avg /= tables_no
			avgCol.append(avg)
		avgTable.insert(0, columns_names[c], avgCol, True)
	print avgTable

	print('Average computed')
	return avgTable

def writeTable(table, filename):
	# table of type pd.DataFrame
	with open(filename, 'w') as f:
		res = table.values  # table to matrix

		s = ''
		for row in res:
			for col in row:
				s += str(col) + ' '
			s += '\n'

		f.write(s)

def getFoldersInForlder(folder):
	return os.listdir(folder)

def updateTicksNo(filename):
	global TICKS_NO
	stringa = os.popen("head -n 25 {}".format(filename)).read()
	m = re.search("simulation_steps=(\d+)", stringa)
	TICKS_NO = int(m.groups()[0])
	print('Ticks of exp:'+str(TICKS_NO))

def process(filename, isPrint=False):
	"""
	Does the work for 1 experiment. Output of the experiment is found in "folder".
	It returns what the experiment would return in the entry as a table
	"""

	# os.system("grep ended {}trace25_mrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > {}resptimes_mrtk.dat".format(folder))
	# os.system("grep ended {}trace25_emrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > {}resptimes_emrtk.dat".format(folder))

	data_res 	= pd.DataFrame({'t':[], 'diff': [], 'over': []})

	with open(filename, 'r') as f:
		for line in f:
			l = line.split(' ')
			time = int(l[0].strip())
			diff = int(l[1].strip())
			over = float(l[2].strip())

			row = pd.DataFrame({'t': [time], 'diff': [diff], 'over': [over]})
			data_res = data_res.append(row)

	return data_res


if __name__ == '__main__':
	# cd rtsim/src/consumption/
	# for((i=0;i<10;i++)); do grep ended ../mrtk/$i/trace25_mrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > ../mrtk/$i/resptimes_mrtk.dat; done
	# for((i=0;i<10;i++)); do grep ended ../energymrtk/$i/trace25_emrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > ../energymrtk/$i/resptimes_emrtk.dat; done

	data_emrtk 	= {}
	data_mrtk 	= {}

	print ('processing emrtk data')
	for i in range(0,len(getFoldersInForlder(FOLDER_ENERGYMRTK))):
		print("processing experiment " + str(i))
		filename = FOLDER_ENERGYMRTK + str(i) + '/resptimes_emrtk.dat'
		updateTicksNo(FOLDER_ENERGYMRTK + str(i) + '/log_emrtk_1_run.txt')
		data_emrtk[i] = process(filename)

	print ('processing mrtk data')
	for i in range(0,len(getFoldersInForlder(FOLDER_MRTK))):
		print("processing experiment " + str(i))
		filename = FOLDER_MRTK + str(i) + '/resptimes_mrtk.dat'
		updateTicksNo(FOLDER_ENERGYMRTK + str(i) + '/log_emrtk_1_run.txt')
		data_mrtk[i] = process(filename)


		#print(data_mrtk)
	data_mrtk 	= computeAverageTableByColumns(data_mrtk)
	data_emrtk 	= computeAverageTableByColumns(data_emrtk)


	print ('writing results to files .dat')
	writeTable(data_emrtk, 	'resptimes_emrtk.dat')
	writeTable(data_mrtk, 	'resptimes_mrtk.dat')

	print ('generating graphs in .PDF')
	os.system("./draw-resptimes.gp")
	#os.system("gv resptimes_emrtk_and_mrtk.pdf")
