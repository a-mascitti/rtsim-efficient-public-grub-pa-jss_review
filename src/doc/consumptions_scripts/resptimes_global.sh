#!/bin/bash

EXPERIMENTS_NO=10

echo "resptimes_global.sh has pwd `pwd`"
p=`pwd`

echo "preparing the single experiments"
for((i=0;i<$EXPERIMENTS_NO;i++)); do
	filename=`basename $p`-$i/consumptions/mrtk/0/resptimes_mrtk.dat
	echo "generating $filename"
	grep ended `basename $p`-$i/consumptions/mrtk/0/trace25_mrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > $filename &
	if [ -s filename ]; then
		echo "$filename not correctly generated. abort"
		exit -1
	fi
done
for((i=0;i<$EXPERIMENTS_NO;i++)); do
	filename=`basename $p`-$i/consumptions/energymrtk/0/resptimes_emrtk.dat
	echo "generating $filename"
	grep ended `basename $p`-$i/consumptions/energymrtk/0/trace25_emrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > $filename &
	if [ -s filename ]; then
		echo "$filename not correctly generated. abort"
		exit -1
	fi
done
wait
echo "You have successfully prepared all the experiments in $p"
echo "--------------------------------"
shopt -s globstar

# take the resptimes of each experiment and append them into a common list (you get a list of resptimes / periods)
echo "global CDF for energymrtk exps"
cat run-**/consumptions/**/resptimes_emrtk.dat | cut -d ' ' -f3 | LC_ALL=C sort -n | less > resptimes_emrtk_all.dat

# same for mrtk experiments
echo "global CDF for mrtk exps"
cat run-**/consumptions/**/resptimes_mrtk.dat | cut -d ' ' -f3 | LC_ALL=C sort -n | less > resptimes_mrtk_all.dat

# plot the 2 resulting lists (only resptimes / periods is considered for y axis)
./resptimes_global.gp