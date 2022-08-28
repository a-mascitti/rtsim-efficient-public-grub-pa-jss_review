#!/usr/bin/python

from argparse import ArgumentParser
from sortedcontainers import SortedDict
import glob, os

def getFilesInFolder(filePattern):
	return glob.glob(filePattern) # att, ~/folder/*.txt non funziona

def printDictionaryToFile(dd, filename):
	if os.path.exists(filename):
		os.remove(filename)

	with open(filename,'w') as file:
		for k in sorted (dd.keys()):
			file.write("%s %s\n" % (k, dd[k]))

def sortDictionaryByKey(dd):
	return sorted(dd.iteritems(), key=lambda key_value: key_value[0])  # ( (key, val), ... )

if __name__ == "__main__":

	parser = ArgumentParser()
	parser.add_argument("-t", "--tick-limit", dest="tickLimit", type=int,
						help="tick limit", default=999999999)
	parser.add_argument("-g", "--granularity", dest="granularity", type=int,
						help="granularity of tick -> W", default=1)
	parser.add_argument("-f", "--file-pattern", dest="filePattern",
						help="file pattern as source")

	args = parser.parse_args()

	tickLimit = args.tickLimit
	granularity = args.granularity
	filePattern = args.filePattern

	if filePattern == "":
		print ("Missing file pattern, dunno what to read")
		exit (1)
	else:
		filePattern = getFilesInFolder(filePattern)
		# print ("Reading files: ")
		# for f in filePattern:
		# 	print (f)
		# print ("")

        print ("tickWatt.py received: " + str(args))

	# prepare intermediate file with t1 -> W...t1+granularity -> W
	# for each filePattern
	# and then make the average, making the final file
	for f in filePattern:
		
		if '_tickWatt_output' in f:
			continue

		with open (f) as fs: content = fs.readlines()
		time = 0
		lastW = 0
		tickWatt = SortedDict()
		for line in content:
			if len(line) <= 0:
				continue
			line  = line.replace('Cur W=','')
			line  = line.replace('bzip2 ','')
			line  = line.replace('idle ','')
			line  = line.replace('t=','')
			line  = line.strip()
			split = line.split(' ')

			# print split
			time  = int(split[1])
			lastW = float(split[0])

			if time > tickLimit: 
				print ("tick limit reached for {}, stopping".format(f))
				break
			# print("(t1-t0)*W => e: ({}-{})*{} => {}".format(newTime, lastTime, lastW, energy))
			
			tickWatt[time] = lastW

		# completing tickWatt with missing ticks
		for i in xrange(0, tickLimit, granularity):
			if i not in tickWatt.keys():
				tickWatt[i] = tickWatt[list(tickWatt.irange(maximum=i))[-1]]

		# applying granularity
		for key, val in tickWatt.items():
			if key % granularity != 0:
				l1= len(tickWatt.keys())
				tickWatt.pop(key)
				assert len(tickWatt.keys()) == l1 -1

		# tickWatt = sortDictionaryByKey(tickWatt)
		printDictionaryToFile(tickWatt, f + "_tickWatt_output.dat")

print ('Thank you, outputs end with _tickWatt_output.dat')
