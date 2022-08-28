#!/usr/bin/python3

import re,sys

def getTime(line):
	m = re.search(r"t=(\d+)", line)
	if m is None:
		return None
	return m.groups()[0]

def getTaskName(line):
	m = re.search(r"PeriodicTask (\w+) DL",line)
	if m is None:
		return None
	return m.groups()[0]

def getTaskInfo(line):
	m = re.search(r"CBS\((.*)\)\.",line)  # abs WCET, P
	if m is None:
		return None
	return m.groups()[0]

def toString(records):
	res = ''
	for k,v in records.items():
		res += k + ":\n"
		for value in v:
			# if 'changeBudget' in s and i+1 < len(values) and 'changeBudget' in values[i+1]:
			# 	continue
			value = value.replace('PN5RTSim25CBServerCallingEMRTKernelE', 'CBSCEMRTK')
			value = value.replace('PN5RTSim8CBServerE', "CBS")
			value = value.replace('putit ', '')
			if " del " in value:
				value = value[:value.index(" del ")]
			res += '\t' + value + '\n'
	return res

if __name__ == "__main__":
	failingTask = ''
	records = {}  # dictionary

	with open('t') as f:
		for line in f:
			# print (line)

			keys = [ 'putit ', 'endEvt', '_bandExEvt', 'onEnd', 'going to schedule' ]
			for k in keys:
				if k in line:
					taskname = getTaskName(line) 
					failingTask = taskname
					if taskname is not None:  # if line contains the task name, store the line, else skip it
						if taskname not in records:
							records[taskname] = []
						records[taskname].append(line)


	towrite = toString(records)

	if len(sys.argv) > 1:
		failingTask = sys.argv[1]
	
	print ("failingTask: " + failingTask)
	towrite += toString({failingTask : records[failingTask]})

	fd_res = open("e", 'w')
	fd_res.write(towrite)
	fd_res.close()
