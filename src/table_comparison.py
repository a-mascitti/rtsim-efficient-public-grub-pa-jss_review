#!/usr/bin/python

import sys

# ts1 freq1
# ts2 freq2
# ts3 freq3

# freq_avg = ( freq1*(ts2-ts1) + freq2*(ts3-ts2) ) / (ts3 - ts1)

if __name__ == '__main__':
	filename = sys.argv[1]

	tsold=0
	tscur=0
	fold =0
	fcur =0
	freq_avg=0
	init=True
	with open(filename, 'r') as f:
		for line in f:
			split = line.split(' ')
			if not init:
				tsold = int(split[0])
				fold  = int(split[1])
				init=False 
				continue
			tscur = int(split[0])
			fcur  = int(split[1])
			freq_avg += (tscur-tsold) * fold
			tsold = tscur
			fold  = fcur

	freq_avg /= tscur
	# print (freq_avg)

	with open('table_comparison_py_output_temp.txt', 'w+') as f:
		f.write(str(freq_avg))
