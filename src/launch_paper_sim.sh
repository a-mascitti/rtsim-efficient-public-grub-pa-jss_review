#!/bin/bash


trap 'exit 0' INT

total_failures=0
total_runtime=0
exp_num=-1

PATH="$PATH:`pwd`/taskset_generator"

usage()
{
	echo -e "$0\t\tExecute 10 paper simulations for each simulation in {$u}.\n$0 -u [util]\tLaunch 10 paper simulations with the specified utlization."
}

while [ "$1" != "" ]; do
	case $1 in
		-h | --help ) 	usage
				exit
				;;
		-u | --util )	shift
				utils=$1
				;;
		-k | --keep-trace )	shift
				keep_trace=$1
				;;
		--exp-num ) 	shift
				exp_num=$1
				;;
		* ) 		usage
				exit 1
	esac
	shift
done

for u in $utils; do
	echo "util $u"
	i=`expr 0`
	# failed_no=`expr 0`
	while [ $i -lt $exp_num ]; do
		echo ""
		echo "Performing exp $i with util $u"

		rm -rf run-$u-$i
		mkdir -p run-$u-$i
		cd run-$u-$i
			ln -s ../data . &> /dev/null
			mkdir -p taskset_generator
			cp `pwd`/../taskset_generator/taskgen3.py ./taskset_generator/
            mkdir -p consumptions
			start=`date +%s`
			../paper_sim --total-utilization $u --texttrace -n > log.txt 2>&1
			ret=$?
			end=`date +%s`
			runtime=$((end-start))
			total_runtime=`expr total_runtime+runtime`

			# since trace file, only needed for CDF, occupies really a lot, we only keep it if needed
			if [ $keep_trace == 0 ]; then
				echo "removing trace files in `pwd`"
				rm -f consumptions/energymrtk/0/trace25_emrtk.txt
				rm -f consumptions/mrtk/0/trace25_mrtk.txt
			fi
    	cd ..

		echo "exp $u no. $i ended returning $ret, completed in $runtime s"
		if [ $ret == 0 ]; then
			i=`expr $i + 1`
			echo "Simulation ended well"
		else
			# failed_no=`expr $failed_no + 1`
			echo "Error in simulation, repeating experiment $i util $u."

		fi
	done
	echo "Experiments for util $u ended."
	# echo "Failed experiments: $failed_no"
done

echo "Simulations performed. Total runtime $total_runtime" > log_launch_paper_sim.txt
