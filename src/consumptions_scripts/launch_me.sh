#!/bin/bash

# first, be sure you have performed all experiments without issues.
# I expect all experiments to be of the same simulation steps.
EXPERIMENTS_NO=1

while [ "$1" != "" ]; do
        case $1 in
                --cdf )   	shift
				cdf=$1
                                ;;
                -u | --util )   shift
                                u=$1
                                ;;
		--exp-num )     shift
				EXPERIMENTS_NO=$1
				;;
                * )             exit 1
        esac
        shift
done


if [[ 1 == $cdf ]]; then
# CDF
echo ""
echo "CDF"
for((i=0;i<$EXPERIMENTS_NO;i++)); do 
	grep ended consumptions/mrtk/$i/trace25_mrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > consumptions/mrtk/$i/resptimes_mrtk.dat; 
	grep ended consumptions/energymrtk/$i/trace25_emrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > consumptions/energymrtk/$i/resptimes_emrtk.dat;
	grep ended consumptions/grub_pa/$i/trace25_mrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > consumptions/grub_pa/$i/resptimes_grubpa.dat;
	grep ended consumptions/energymrtk_bf/$i/trace25_emrtk_bf.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > consumptions/energymrtk_bf/$i/resptimes_emrtk_bf.dat;
	grep ended consumptions/energymrtk_ff/$i/trace25_emrtk_ff.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > consumptions/energymrtk_ff/$i/resptimes_emrtk_ff.dat;
	./resptimes.sh
done
fi

# frequencies over time & power consumptions
echo ""
echo "power consumptions for `pwd`"
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

hyperp=`cat consumptions/energymrtk/0/log_emrtk_*.txt | grep 'simulation_steps=' | tail -n1 | sed 's/simulation_steps=//'`
if [[ `wc -l consumptions/energymrtk_bf/0/freqBIG.txt | sed 's/ .*//'` == 0 ]]; then echo -e "0 big 200\n$hyperp big 200" > consumptions/energymrtk_bf/0/freqBIG.txt; fi
if [[ `wc -l consumptions/energymrtk_bf/0/freqLITTLE.txt | sed 's/ .*//'` == 0 ]]; then echo -e "0 LITTLE 200\n$hyperp LITTLE 200" > consumptions/energymrtk_bf/0/freqLITTLE.txt; fi
if [[ `wc -l consumptions/energymrtk_ff/0/freqBIG.txt | sed 's/ .*//'` == 0 ]]; then echo -e "0 big 200\n$hyperp big 200" > consumptions/energymrtk_ff/0/freqBIG.txt; fi
if [[ `wc -l consumptions/energymrtk_ff/0/freqLITTLE.txt | sed 's/ .*//'` == 0 ]]; then echo -e "0 LITTLE 200\n$hyperp LITTLE 200" > consumptions/energymrtk_ff/0/freqLITTLE.txt; fi

python2 ./freq.py  --only-one


# power -> ticks cdf
./pow_ticks.py
./pow_ticks.gp