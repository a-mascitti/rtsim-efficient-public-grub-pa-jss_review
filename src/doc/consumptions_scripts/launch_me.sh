#!/bin/bash

# first, be sure you have performed all experiments without issues.
# I expect all experiments to be of the same simulation steps.
EXPERIMENTS_NO=1


# CDF
echo ""
echo "CDF"
for((i=0;i<$EXPERIMENTS_NO;i++)); do 
	grep ended consumptions/mrtk/$i/trace25_mrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > consumptions/mrtk/$i/resptimes_mrtk.dat; 
done
for((i=0;i<$EXPERIMENTS_NO;i++)); do 
	grep ended consumptions/energymrtk/$i/trace25_emrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > consumptions/energymrtk/$i/resptimes_emrtk.dat;
done
cd resptimes/
./resptimes.sh
cd ../

# frequencies over time & power consumptions
echo ""
echo "power consumptions"
python2 graph.py

# time needed by placement algorithm
echo ""
echo "placement times"
cd placement_time/
python2 placement_time.py

# frequencies
echo ""
echo "frequencies"
cd ..
python2 ./freq.py  --only-one
