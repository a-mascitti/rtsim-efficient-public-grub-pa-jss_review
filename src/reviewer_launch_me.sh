#!/bin/bash


sequenceContains() {
  sequ=$1
  elem=$2
  for e in $sequ; do
    if [ $e == $elem ]; then
        return 1
    fi
  done
  return 0
}

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT
#3set -x

#utils="0.25 0.4 0.5 0.65"
#utils="0.2 0.3 0.35 0.45 0.55 0.6 0.7"
utils="0.2 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.6 0.65 0.7"
exp_num=10

keep_traces="0.25 0.4 0.5 0.65"

is_run_script="no"
while [ "$1" != "" ]; do
        case $1 in
                -y )		shift
                                is_run_script="yes"
                                ;;
		-n ) 		shift
				is_run_script="no"
				;;
                * )             echo "Cmd line option unrecognized"
                                exit 1
        esac
        shift
done

echo "----------------------------------- reviewer_launch_me.sh"

echo "For this script to work, you need to have 10 non-aborting/killed experiments for each utilization in {$utils}."
echo -n "Do you want the script to run 10 test for each utilization in { $utils }? [y/n; default=y] "
if [[ $is_run_script == *"y"* ]]; then
	echo "Got cmd line option to perform experiments"
else
	if [[ $is_run_script != "no" ]]; then
		read is_run_script
	fi
fi

if [[ "$is_run_script" == *"y"* ]] ; then
	for u in $utils; do

		# determine if trace file must be deleted (since it occupies much HDD)
		keep_trace=0
		for k in $keep_traces; do
			if [[ $k == $u ]]; then
				keep_trace=1
			fi
		done
		./launch_paper_sim.sh -u $u -k $keep_trace --exp-num $exp_num &
	done
	wait
fi

echo "Checking prerequirements for making graphs.."
shopt -s globstar
allok=1
for u in $utils; do
	for((i=0;i<$exp_num;i++)); do
		for f in run-$u-$i/**/log_*; do
			echo -n "Checking $f. "

			if [[ `tail $f | grep "Simulation finished" | wc -l` == 0 ]]; then
				echo "Managed error, $f has not finished well. Please repeat experiment manually with:"
				echo "cd $f && ../paper_sim -n && cd .. && $0"
				allok=0
			else
				echo "0K"
			fi
		done
	done
done

if [[ $allok == 0 ]]; then
	echo "Some exp failed. Please repeat them first as indicated above."
	exit 1
fi

echo "------------------------------ Checking hyperperiods"
hyperperiods_fn='hyperperiods.dat'
rm -f $hyperperiods_fn
for u in $utils; do
	for ((i=0;i<$exp_num;i++)); do
		emrtk=0
		grubpa=0
		mrtk=0
		for f in run-$u-$i/; do
			echo -n "Checking hyperperiod $f. "
			emrtk=`cat $f/consumptions/energymrtk/0/log_emrtk_*.txt | grep 'simulation_steps=' | tail -n1 | sed 's/simulation_steps=//'`
			mrtk=`cat $f/consumptions/mrtk/0/log_mrtk_*.txt | grep 'Simulation steps:' | tail -n1 | sed 's/Simulation steps: //'`
			grubpa=`cat $f/consumptions/grub_pa/0/log_mrtk_*.txt | grep "Simulation steps:" | tail -n1 | sed 's/Simulation steps: //'`
			echo "$u $i $emrtk $mrtk $grubpa" >> $hyperperiods_fn
			if [ $emrtk -eq $mrtk ] && [ $emrtk -eq $grubpa ]; then
				echo "0K"
			else
				"Managed error, $f has different hyperperiods"
				exit 1
			fi
		done
	done
done

echo "graph tick -> freq and tick -> W for big and LITTLE, 1 experiment only..."
for u in $utils; do
	for((i=0;i<$exp_num;i++)); do
		echo "---------------------------------"
		echo "processing run-$u-$i/"
		cp -r consumptions_scripts/* run-$u-$i/
		cd run-$u-$i/
		echo "./launch_me.sh -u $u --cdf $( sequenceContains "$keep_traces" $u ; echo $? )"
		./launch_me.sh -u $u --cdf $( sequenceContains "$keep_traces" $u ; echo $? ) & # CDF, freq and power over time for 1 exp
		cd -
	done
done
wait

echo "----------------------------------- reviewer_launch_me.sh"
echo "Now you should have for each util and each exp their own graphs and data. So you can merge the data"

echo "Now making global CDF..."
for u in $utils; do
	mkdir -p run-$u/
	for((i=0;i<$exp_num;i++)); do
		ln -f -s `pwd`/run-$u-$i/ run-$u/
	done

	if [[ 1 == $( sequenceContains "$keep_traces" $u ; echo $? ) ]]; then
	echo "global CDF (requires you to have 10 exp with cores util $u)..."
	cd run-$u/
		cp run-$u-0/resptimes_global.sh .
		cp run-$u-0/resptimes_global.gp .
		./resptimes_global.sh --exp-num $exp_num
	cd ..
	fi

	echo ""
	echo "average and variance of energies and frequencies..."
	cd run-$u/
	cat ../run-$u-*/graph_py_output.json | grep 'avg energy consumption big emrtk\"' | sed -e 's/.*: //;s/,//' > avg_energy_consumption_big_emrtk.dat
	cat ../run-$u-*/graph_py_output.json | grep 'avg energy consumption little emrtk\"' | sed -e 's/.*: //;s/,//' > avg_energy_consumption_little_emrtk.dat
	cat ../run-$u-*/graph_py_output.json | grep 'avg energy consumption big mrtk' | sed -e 's/.*: //;s/,//' > avg_energy_consumption_big_mrtk.dat
	cat ../run-$u-*/graph_py_output.json | grep 'avg energy consumption little mrtk' | sed -e 's/.*: //;s/,//' > avg_energy_consumption_little_mrtk.dat
	cat ../run-$u-*/graph_py_output.json | grep 'avg energy consumption big grubpa' | sed -e 's/.*: //;s/,//' > avg_energy_consumption_big_grubpa.dat
	cat ../run-$u-*/graph_py_output.json | grep 'avg energy consumption little grubpa' | sed -e 's/.*: //;s/,//' > avg_energy_consumption_little_grubpa.dat
	cat ../run-$u-*/graph_py_output.json | grep 'avg energy consumption big emrtk_bf' | sed -e 's/.*: //;s/,//' > avg_energy_consumption_big_emrtk_bf.dat
	cat ../run-$u-*/graph_py_output.json | grep 'avg energy consumption little emrtk_bf' | sed -e 's/.*: //;s/,//' > avg_energy_consumption_little_emrtk_bf.dat
	cat ../run-$u-*/graph_py_output.json | grep 'avg energy consumption big emrtk_ff' | sed -e 's/.*: //;s/,//' > avg_energy_consumption_big_emrtk_ff.dat
	cat ../run-$u-*/graph_py_output.json | grep 'avg energy consumption little emrtk_ff' | sed -e 's/.*: //;s/,//' > avg_energy_consumption_little_emrtk_ff.dat
	
	cat ../run-$u-*/freq_py_output_exp0.json | grep 'avg freq big emrtk\"' | sed -e 's/.*: //;s/,//' > avg_frequency_big_emrtk.dat
	cat ../run-$u-*/freq_py_output_exp0.json | grep 'avg freq LITTLE emrtk\"' | sed -e 's/.*: //;s/,//' > avg_frequency_little_emrtk.dat
	cat ../run-$u-*/freq_py_output_exp0.json | grep 'avg freq big grubpa' | sed -e 's/.*: //;s/,//' > avg_frequency_big_grubpa.dat
	cat ../run-$u-*/freq_py_output_exp0.json | grep 'avg freq LITTLE grubpa' | sed -e 's/.*: //;s/,//' > avg_frequency_little_grubpa.dat
	cat ../run-$u-*/freq_py_output_exp0.json | grep 'avg freq big emrtk_bf' | sed -e 's/.*: //;s/,//' > avg_frequency_big_emrtk_bf.dat
	cat ../run-$u-*/freq_py_output_exp0.json | grep 'avg freq LITTLE emrtk_bf' | sed -e 's/.*: //;s/,//' > avg_frequency_little_emrtk_bf.dat
	cat ../run-$u-*/freq_py_output_exp0.json | grep 'avg freq big emrtk_ff' | sed -e 's/.*: //;s/,//' > avg_frequency_big_emrtk_ff.dat
	cat ../run-$u-*/freq_py_output_exp0.json | grep 'avg freq LITTLE emrtk_ff' | sed -e 's/.*: //;s/,//' > avg_frequency_little_emrtk_ff.dat
	cd ..
done 
python ./energies_utilizations.py $exp_num
python ./frequencies_utilizations.py $exp_num


echo "-------------------------------- reviewer_launch_me.sh"
rm -f scatter_energy.dat scatter_energy_ratio.dat energy_saving_percent.dat
echo "U kernel total_energy_nJ exp_i" > scatter_energy.dat
echo "U GRUBPA BL-CBS saving_perc exp_i" > energy_saving_percent.dat
echo "U tot_energy_nJ_GRUB_PA/tot_energy_nJ_BL_CBS tot_energy_nJ_G_EDF/tot_energy_nJ_BL_CBS exp_i" > scatter_energy_ratio.dat
for u in $utils; do
	for ((i=0;i<$exp_num;i++)); do
		energy_emrtk=`cat run-$u-$i/graph_py_output.json | grep 'paper alg big + little' | sed -e 's/.*: //;s/,//'`
		energy_grubpa=`cat run-$u-$i/graph_py_output.json | grep 'GRUBPA big + little' | sed -e 's/.*: //;s/,//'`
		energy_edf=`cat run-$u-$i/graph_py_output.json | grep 'EDF big + little' | sed -e 's/.*: //;s/,//'`
		energy_edf_bf=`cat run-$u-$i/graph_py_output.json | grep 'BF big + little' | sed -e 's/.*: //;s/,//'`
		energy_edf_ff=`cat run-$u-$i/graph_py_output.json | grep 'FF big + little' | sed -e 's/.*: //;s/,//'`
		ratio_grubpa_emrtk=`python -c "print float('$energy_grubpa') / float('$energy_emrtk')"`  # hyperperiod-independent
		ratio_gedf_emrtk=`python -c "print float('$energy_edf') / float('$energy_emrtk')"`
		ratio_bf_emrtk=`python -c "print float('$energy_edf_bf') / float('$energy_emrtk')"`
		ratio_ff_emrtk=`python -c "print float('$energy_edf_ff') / float('$energy_emrtk')"`
		energy_saving_perc=`python -c "print round( ( float('$energy_grubpa') - float('$energy_emrtk') ) / float('$energy_grubpa') * 100, 4 )"`
		echo "$u BL-CBS $energy_emrtk $i" >> scatter_energy.dat
		echo "$u GRUBPA $energy_grubpa $i" >> scatter_energy.dat
		echo "$u $energy_grubpa $energy_emrtk $energy_saving_perc $i" >> energy_saving_percent.dat
		echo "$u $ratio_grubpa_emrtk $ratio_gedf_emrtk $ratio_bf_emrtk $ratio_ff_emrtk $i" >> scatter_energy_ratio.dat
	done
done

energy_saving_perc=`cat energy_saving_percent.dat | sed '1d' | sed '$ d'| cut -d ' ' -f 6 | datastat --min --max --dev`
energy_saving_perc_no_neg=`cat energy_saving_percent.dat | sed '1d' | sed '$d'| cut -d ' ' -f 6 | sed '/^-/d' | datastat --min --max --dev`
echo "energy saving perc: " >> energy_saving_percent.dat
echo "$energy_saving_perc" >> energy_saving_percent.dat
echo "energy saving without negatives: " >> energy_saving_percent.dat
echo "$energy_saving_perc_no_neg" >> energy_saving_percent.dat

rm -f scatter_energy_ratio_avg.dat
echo "U avg_tot_energy_nJ_GRUB_PA/tot_energy_nJ_BL_CBS min max avg_tot_energy_nJ_G_EDF/tot_energy_nJ_BL_CBS min max avg_tot_energy_nJ_EDF_BF/tot_energy_nJ_BL_CBS min max avg_tot_energy_nJ_EDF_FF/tot_energy_nJ_BL_CBS min max" > scatter_energy_ratio_avg.dat
for u in $utils; do
	a=`cat scatter_energy_ratio.dat | grep "^$u " | cut -d ' ' -f 2 | datastat --no-header --min --max`
	b=`cat scatter_energy_ratio.dat | grep "^$u " | cut -d ' ' -f 3 | datastat --no-header --min --max`
	c=`cat scatter_energy_ratio.dat | grep "^$u " | cut -d ' ' -f 4 | datastat --no-header --min --max`
	d=`cat scatter_energy_ratio.dat | grep "^$u " | cut -d ' ' -f 5 | datastat --no-header --min --max`
	echo "$u $a $b $c $d" >> scatter_energy_ratio_avg.dat
done

gnuplot ./scatter_energy.gp


rm -f errorbar_energy.dat
echo "U avg dev kernel" > errorbar_energy.dat
for u in $utils; do
	for ((i=0;i<$exp_num;i++)); do
		for kernel in GRUBPA BL-CBS; do
			dev=`cat scatter_energy.dat | grep "$u " | grep $kernel | cut -d ' ' -f 3 | datastat -nh --dev --no-avg`
			avg=`cat scatter_energy.dat | grep "$u " | grep $kernel | cut -d ' ' -f 3 | datastat -nh`
			echo "$u $avg $dev $kernel" >> errorbar_energy.dat
		done
	done	
done

gnuplot ./errorbar_energy.gp


echo "--------------------------------- $0"
rm -f ./migrations.dat
echo "U migr_emrtk migr_bf migr_ff migr_grubpa migr_gedf" > migrations.dat
for u in $utils; do
	migr_emrtk=`tail -n 1000 run-$u-*/consumptions/energymrtk/0/log_emrtk_1_run.txt | grep migrations | sed -e 's/.*migrations: //;s/, total.*//' | datastat -nh`
	migr_bf=`tail -n 1000 run-$u-*/consumptions/energymrtk_bf/0/log_emrtk_1_run.txt | grep migrations | sed -e 's/.*migrations: //;s/, total.*//' | datastat -nh`
	migr_ff=`tail -n 1000 run-$u-*/consumptions/energymrtk_ff/0/log_emrtk_1_run.txt | grep migrations | sed -e 's/.*migrations: //;s/, total.*//' | datastat -nh`
	migr_grubpa=`tail -n 1000 run-$u-*/consumptions/grub_pa/0/log_mrtk_1_run.txt | grep migrations | sed -e 's/.*migrations: //;s/, total.*//' | datastat -nh`
	migr_gedf=`tail -n 1000 run-$u-*/consumptions/mrtk/0/log_mrtk_1_run.txt | grep migrations | sed -e 's/.*migrations: //;s/, total.*//' | datastat -nh`
	echo "$u $migr_emrtk $migr_bf $migr_ff $migr_grubpa $migr_gedf" >> migrations.dat
done
./migrations.gp

echo "--------------------------------- reviewer_launch_me.sh"
echo "now checking against paper propositions"
for u in $utils; do
	for((i=0;i<$exp_num;i++)); do
		cp consumptions_scripts/check_proposition.py run-$u-$i/
		cd run-$u-$i
		./check_proposition.py &
		cd ..
	done
done
wait

rm -f aggregate_check_propositions.dat
echo "U %adm theor_name" > aggregate_check_propositions.dat
for u in $utils; do
	# prop1true=`cat run-$u-*/checkAgainstPaperPropositions.txt | grep "prop\. 1.*true" | wc -l`
	# prop2true=`cat run-$u-*/checkAgainstPaperPropositions.txt | grep "prop\. 2.*true" | wc -l`
	theorem1=`cat run-$u-*/check_proposition.dat | grep "theorem 1.*true" | wc -l`
	perc=`echo "scale=4; $theorem1 / $exp_num.0 * 100" | bc`
	echo "$u $perc theorem1" >> aggregate_check_propositions.dat
done
gnuplot ./check_proposition.gp


echo "---------------------------------- reviewer_launch_me.sh"
echo "placement times"
rm -f placement.dat
echo "max values for each exp (us):" > placement.dat
cat run-*/*/placement_time_py_output.json | grep "max" | sed 's/.*max": //g;s/,//' >> placement.dat
echo "---------------------" >> placement.dat
echo "mean values for each exp (us):" >> placement.dat
cat run-*/*/placement_time_py_output.json | grep "mean" | sed 's/.*mean": //g;s/,//' >> placement.dat
max_pl=`cat run-*/*/placement_time_py_output.json | grep "max" | sed 's/.*max": //g;s/,//' | xargs printf "%.4f\n" | sed 's/,/./' | datastat -nh`
mean_pl=`cat run-*/*/placement_time_py_output.json | grep "mean" | sed 's/.*mean": //g;s/,//' | xargs printf "%.4f\n" | sed 's/,/./' | datastat -nh`
echo "mean (us): $mean_pl; max (us): $max_pl" >> placement.dat

echo "---------------------------------- reviewer_launch_me.sh"
echo "making dl miss graph"
./dl.sh "$utils" $exp_num

echo "---------------------------------- reviewer_launch_me.sh"
echo "making tick -> energy for every tick for u={0.25, 0.5}"
for u in 0.25 0.5; do
        for((i=0;i<$exp_num;i++)); do
		echo "cd run-$u-$i/"
		cd run-$u-$i/
		# 1.000.000 ns = 1 ms
		./tickWatt.py --granularity 500000 --tick-limit 1000000000 --file-pattern consumptions/energymrtk/0/powerLITTLE.txt
		./tickWatt.py --granularity 500000 --tick-limit 1000000000 --file-pattern consumptions/energymrtk/0/powerBIG.txt
		./tickWatt.py --granularity 500000 --tick-limit 1000000000 --file-pattern consumptions/mrtk/0/powerLITTLE.txt
		./tickWatt.py --granularity 500000 --tick-limit 1000000000 --file-pattern consumptions/mrtk/0/powerBIG.txt
		./tickWatt.py --granularity 500000 --tick-limit 1000000000 --file-pattern consumptions/grub_pa/0/powerLITTLE.txt
		./tickWatt.py --granularity 500000 --tick-limit 1000000000 --file-pattern consumptions/grub_pa/0/powerBIG.txt
		./energy.gp
		cd ..
	done
done

echo "---------------------------------- reviewer_launch_me.sh"
echo "making paper table"
./table_comparison.sh --exp-num $exp_num

echo "Thank you!"
