#!/bin/bash

utils="0.7"

nMissingTot=0
nInstancesTot=0

for u in $utils; do
	for n in `seq 0 9`; do
		echo run-$u-$n/ 
		cd run-$u-$n/
			hyperperiod=`head consumptions/energymrtk/0/log_emrtk_1_run.txt | grep 'simulation_steps=' | sed 's/simulation_steps=//'`
			periods=`cat log.txt | grep ").*wcet" | sed 's/.*period //g'`
			
			missingHere=`tail -n20 consumptions/energymrtk/0/log_emrtk_1_run.txt | grep "missing tasks: " | sed 's/.*missing tasks: //;s/, not.*//'`
			nMissingTot=`python -c "print $nMissingTot+$missingHere"`

			for p in $periods; do
				echo "$nInstancesTot+($hyperperiod / $p)"
				nInstancesTot=`python -c "print $nInstancesTot+($hyperperiod / $p)"`
				echo $nInstancesTot
			done
		cd ..
	done
	rm -f num_instances_output.dat
	echo -e "missingTot u=$u: $nMissingTot \nInstancesTot u=$u: $nInstancesTot" > num_instances_output.dat
	echo "Results saved in num_instances_output.dat"
done
