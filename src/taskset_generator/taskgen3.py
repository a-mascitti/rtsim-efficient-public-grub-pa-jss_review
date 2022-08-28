#!/usr/bin/python3

"""A taskset generator for experiments with real-time task sets

Copyright 2010 Paul Emberson, Roger Stafford, Robert Davis. 
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

	 1. Redistributions of source code must retain the above copyright notice, 
			this list of conditions and the following disclaimer.

	 2. Redistributions in binary form must reproduce the above copyright notice,
			this list of conditions and the following disclaimer in the documentation 
			and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY EXPRESS 
OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO 
EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are 
those of the authors and should not be interpreted as representing official 
policies, either expressed or implied, of Paul Emberson, Roger Stafford or 
Robert Davis.

Includes Python implementation of Roger Stafford's randfixedsum implementation
http://www.mathworks.com/matlabcentral/fileexchange/9700
Adapted specifically for the purpose of taskset generation with fixed
total utilisation value

Please contact paule@rapitasystems.com or robdavis@cs.york.ac.uk if you have 
any questions regarding this software.
"""

import numpy
import optparse
import sys
import textwrap

import math
import os

def StaffordRandFixedSum(n, u, nsets):

		#deal with n=1 case
		if n == 1:
				return numpy.tile(numpy.array([u]),[nsets,1])
		
		k = numpy.floor(u)
		s = u
		step = 1 if k < (k-n+1) else -1
		s1 = s - numpy.arange( k, (k-n+1)+step, step )
		step = 1 if (k+n) < (k-n+1) else -1
		s2 = numpy.arange( (k+n), (k+1)+step, step ) - s

		tiny = numpy.finfo(float).tiny
		huge = numpy.finfo(float).max

		w = numpy.zeros((n, n+1))
		w[0,1] = huge
		t = numpy.zeros((n-1,n))

		for i in numpy.arange(2, (n+1)):
				tmp1 = w[i-2, numpy.arange(1,(i+1))] * s1[numpy.arange(0,i)]/float(i)
				tmp2 = w[i-2, numpy.arange(0,i)] * s2[numpy.arange((n-i),n)]/float(i)
				w[i-1, numpy.arange(1,(i+1))] = tmp1 + tmp2;
				tmp3 = w[i-1, numpy.arange(1,(i+1))] + tiny;
				tmp4 = numpy.array( (s2[numpy.arange((n-i),n)] > s1[numpy.arange(0,i)]) )
				t[i-2, numpy.arange(0,i)] = (tmp2 / tmp3) * tmp4 + (1 - tmp1/tmp3) * (numpy.logical_not(tmp4))

		m = nsets
		x = numpy.zeros((n,m))
		rt = numpy.random.uniform(size=(n-1,m)) #rand simplex type
		rs = numpy.random.uniform(size=(n-1,m)) #rand position in simplex
		s = numpy.repeat(s, m);
		j = numpy.repeat(int(k+1), m);
		sm = numpy.repeat(0, m);
		pr = numpy.repeat(1, m);

		for i in numpy.arange(n-1,0,-1): #iterate through dimensions
				e = ( rt[(n-i)-1,...] <= t[i-1,j-1] ) #decide which direction to move in this dimension (1 or 0)
				sx = rs[(n-i)-1,...] ** (1/float(i)) #next simplex coord
				sm = sm + (1-sx) * pr * s/float(i+1)
				pr = sx * pr
				x[(n-i)-1,...] = sm + pr * e
				s = s - e
				j = j - e #change transition table column if required

		x[n-1,...] = sm + pr * s
		
		#iterated in fixed dimension order but needs to be randomised
		#permute x row order within each column
		for i in range(0,m):
				x[...,i] = x[numpy.random.permutation(n),i]

		return numpy.transpose(x);

def gen_periods_2_3_5(n, nsets, min, max, gran, max_lcm, hasPrint=False):
	###
	# The problem of gen_periods() is that the correspondent 
	# hyperperiod is huge (more than a billion) and thus simulations
	# take very much time. Since hyperperiod=lcm, the idea is that 
	# periods factors should be only 2, 3 and 5.
	# 
	# Taskgen.py generates first tasks utilization and then their periods.
	# This function makes periods = 2^a * 3^b * 5^c, where a, b and c are
	# randomly generated between 0 and given maximum for each component
	# (i.e., the maximum values) and 2^a_max * 3^b_max * 5^c_max = period_max
	###
	if not hasPrint:
		sys.stdout = open(os.devnull, 'w')
	a_max = 4  # my pick
	b_max = 4  # my pick
	c_max = numpy.ceil( math.log(max / (2**a_max * 3**b_max), 5) )
	print("c=" + str(c_max))
	assert c_max > 0
	periods = []
	for nset in range(0, nsets):
		taskno = 0
		subset_periods = []
		while taskno < n:
			a = numpy.random.randint(low=0, high=a_max + 1, size=1)
			b = numpy.random.randint(low=0, high=b_max + 1, size=1)
			c = numpy.random.randint(low=0, high=c_max + 1, size=1)
			period = int(2**a * 3**b * 5**c)
			
			if period < min or period > max:
				continue
			print ("a {} b {} c {}".format(a,b,c))

			subset_periods.append(period)
			taskno += 1
		periods.append(subset_periods)

	periods = numpy.array(periods)
	print(periods)
	assert periods.shape[0] * periods.shape[1] == n * nsets

	print("max_lcm={}".format(max_lcm))
	periods = numpy.floor(periods / gran) * gran
	print("periods={}, flattened={}".format(periods, periods.flatten().astype(int) ))
	if max_lcm != -1:
			lcm = numpy.lcm.reduce(periods.flatten().astype(numpy.int64))
			if lcm > max_lcm:
				print("LCM of generated tasks is {} while maximum allowed is {}. Repeating generation...".format(lcm, max_lcm))
				periods = gen_periods_2_3_5(n, nsets, min, max, gran, max_lcm, hasPrint)
			else:
				print("successfully generated periods list with LCM {}".format(lcm))
				os.system("echo {} > taskgen_output".format(lcm))

	sys.stdout = sys.__stdout__

	return periods

def gen_periods(n, nsets, min, max, gran, dist, max_lcm):

		if dist == "logunif":
				periods = numpy.exp(numpy.random.uniform(low=numpy.log(min), high=numpy.log(max+gran), size=(nsets,n)))
		elif dist == "unif":
				periods = numpy.random.uniform(low=min, high=(max+gran), size=(nsets,n))
		elif dist == "235":
			periods = gen_periods_2_3_5(n, nsets, min, max, gran, max_lcm, False)
		else:
				return None
		periods = numpy.floor(periods / gran) * gran

		return periods

def gen_tasksets(options):
		x = StaffordRandFixedSum(options.n, options.util, options.nsets)
		periods = gen_periods(options.n, options.nsets, options.permin, options.permax, options.pergran, options.perdist, options.max_lcm)
		#iterate through each row (which represents utils for a taskset)
		for i in range(numpy.size(x, axis=0)):
				C = x[i] * periods[i]
				if options.round_C:
						C = numpy.round(C, decimals=0)
				
				taskset = numpy.c_[x[i], C / periods[i], periods[i], C]

				print_taskset(taskset, options.format)
				if (i < numpy.size(x, axis=0) - 1):
					print ("")
		
def print_taskset(taskset, format):
		for t in range(numpy.size(taskset,0)):
				data = { 'Ugen' : taskset[t][0], 'U' : taskset[t][1], 'T' : taskset[t][2], 'C' : taskset[t][3] }
				print (format % data, end='')

def main():

		usage_str = "%prog [options]"

		description_str = "This is a taskset generator intended for generating data for experiments with real-time schedulability tests and design space exploration tools.  The utilisation generation is done using Roger Stafford's randfixedsum algorithm.  A paper describing this tool was published at the WATERS 2010 workshop. Copyright 2010 Paul Emberson, Roger Stafford, Robert Davis. All rights reserved.  Run %prog --about for licensing information."
						

		epilog_str = "Examples:"
 
		#don't add help option as we will handle it ourselves
		parser = optparse.OptionParser(usage=usage_str, 
																	 description=description_str,
																	 epilog=epilog_str,
																	 add_help_option=False,
																	 version="%prog version 0.1")
	 
		parser.add_option("-h", "--help", action="store_true", dest="help",
											default=False,
											help="Show this help message and exit")

		parser.add_option("--about", action="store_true", dest="about",
											default=False,
											help="See licensing and other information about this software")
						
		parser.add_option("-u", "--taskset-utilisation",
											metavar="UTIL", type="float", dest="util",
											default="0.75",
											help="Set total taskset utilisation to UTIL [%default]")
		parser.add_option("-n", "--num-tasks",
											metavar="N", type="int", dest="n",
											default="5",
											help="Produce tasksets of size N [%default]")
		parser.add_option("-s", "--num-sets",
											metavar="SETS", type="int", dest="nsets",
											default="3",
											help="Produce SETS tasksets [%default]")
		parser.add_option("-S", "--seed",
											metavar="SEED", type="int", dest="seed",
											default="0",
											help="Set the random number generator seed [%default]")
		parser.add_option("-d", "--period-distribution",
											metavar="PDIST", type="string", dest="perdist",
											default="logunif",
											help="Choose period distribution to be 'unif' or 'logunif' [%default]")
		parser.add_option("-p", "--period-min",
											metavar="PMIN", type="int", dest="permin",
											default="1000",
											help="Set minimum period value to PMIN [%default]")
		parser.add_option("-q", "--period-max",
											metavar="PMAX", type="int", dest="permax",
											default=None,
											help="Set maximum period value to PMAX [PMIN]")
		parser.add_option("-g", "--period-gran",
											metavar="PGRAN", type="int", dest="pergran",
											default=None,
											help="Set period granularity to PGRAN [PMIN]")
		
		parser.add_option("--round-C", action="store_true", dest="round_C",
											default=False,
											help="Round execution times to nearest integer [%default]")

		parser.add_option("--max-lcm", type="int", dest="max_lcm",
											default=-1,
											help="Optional and only for -d 235. Max LCM of periods (for simulations) allowed [%default]")
		
		format_default = '%(Ugen)f %(U)f %(C).2f %(T)d\\n';
		format_help = "Specify output format as a Python template string.  The following variables are available: Ugen - the task utilisation value generated by Stafford's randfixedsum algorithm, T - the generated task period value, C - the generated task execution time, U - the actual utilisation equal to C/T which will differ from Ugen if the --round-C option is used.  See below for further examples.  A new line is always inserted between tasksets. [" + format_default + "]"
		parser.add_option("-f", "--output-format",
											metavar="FORMAT", type="string", dest="format",
											default = '%(Ugen)f %(U)f %(C).2f %(T)d\n',
											help=format_help)

		(options, args) = parser.parse_args()

		if options.about:
				print(__doc__)
				return 0

		if options.help:
				print_help(parser)
				return 0

		if options.n < 1:
				sys.stderr.write("Minimum number of tasks is 1")
				return 1

		if options.util > options.n:
				sys.stderr.write("Taskset utilisation must be less than or equal to number of tasks")
				return 1

		if options.nsets < 1:
				sys.stderr.write("Minimum number of tasksets is 1")
				return 1

		if options.seed > 0:
				sys.stderr.write("Setting the seed to " + str(options.seed))
				numpy.random.seed(options.seed)

		known_perdists = ["unif", "logunif", "235"]
		if options.perdist not in known_perdists:
				sys.stderr.write("Period distribution must be one of " + str(known_perdists))
				return 1

		if options.permin <= 0:
				sys.stderr.write("Period minimum must be greater than 0")
				return 1

		#permax = None is default.  Set to permin in this case
		if options.permax == None:
				options.permax = options.permin

		if options.permin > options.permax:
				sys.stderr.write("Period maximum must be greater than or equal to minimum")
				return 1

		#pergran = None is default.  Set to permin in this case
		if options.pergran == None:
				options.pergran = options.permin
				
		if options.pergran < 1:
				sys.stderr.write("Period granularity must be an integer greater than equal to 1")
				return 1

		if (options.permax % options.pergran) != 0:
				sys.stderr.write("Period maximum must be a integer multiple of period granularity")
				return 1

		if (options.permin % options.pergran) != 0:
				sys.stderr.write("Period minimum must be a integer multiple of period granularity")
				return 1
				
		options.format = options.format.replace("\\n", "\n")

		gen_tasksets(options)
		
		return 0
		
def print_help(parser):
		parser.print_help();

		print("")
		
		example_desc = \
						"Generate 5 tasksets of 10 tasks with loguniform periods " +\
						"between 1000 and 100000.  Round execution times and output "+\
						"a table of execution times and periods."
		print(textwrap.fill(example_desc, 75))
		print("    " +parser.get_prog_name() + " -s 5 -n 10 -p 1000 -q 100000 -d logunif --round-C -f \"%(C)d %(T)d\\n\"")

		print("")

		example_desc = \
						"Print utilisation values from Stafford's randfixedsum " +\
						"for 20 tasksets of 8 tasks, with one line per taskset, " +\
						"rounded to 3 decimal places:"

		print(textwrap.fill(example_desc, 75))
		print("    " + parser.get_prog_name() + " -s 20 -n 8 -f \"%(Ugen).3f\"")

if __name__ == "__main__":
		sys.exit(main())

