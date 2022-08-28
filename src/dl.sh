#!/bin/bash

utils=$1
exp_num=$2
echo "Got utils=$utils"
echo "Got exp_num=$exp_num"

fname=dl_sh_output.dat
rm -f $fname

#set -x

echo "util & tot_missing_tasks_10_exp_emrtk & tot_missing_tasks_10_exp_mrtk & tot_missing_tasks_10_exp_grub_pa & perc_tot_dl_miss_emrtk & perc_tot_dl_miss_mrtk & perc_tot_dl_miss_grub_pa \\\\" >> $fname

for u in $utils; do
	echo $u
	tot_missing_tasks_10_exp_emrtk=`tail run-$u-*/consumptions/energymrtk/0/log_emrtk_1_run.txt | grep "missing tasks:" | sed "s/.*missing tasks: //g;s/, not schedulable tasks.*//g;" | datastat --sum --no-avg --no-header`
	tot_missing_tasks_10_exp_mrtk=`tail run-$u-*/consumptions/mrtk/0/log_mrtk_1_run.txt | grep "missing tasks:" | sed "s/.*missing tasks: //g;" | datastat --sum --no-avg --no-header`
	tot_missing_tasks_10_exp_grub_pa=`tail run-$u-*/consumptions/grub_pa/0/log_mrtk_1_run.txt | grep "missing tasks:" | sed "s/.*missing tasks: //g;s/,.*//g" | datastat --sum --no-avg --no-header`


	# we work with the same taskset on all kernels
	perc_tot_dl_miss_emrtk=0  # total ratio of missing tasks emrtk
	perc_tot_dl_miss_mrtk=0
	perc_tot_dl_miss_grub_pa=0
	for ((i=0;i<$exp_num;i++)); do
		hyp=`grep steps run-$u-$i/consumptions/energymrtk/0/log_emrtk_1_run.txt | sed 's/.*=//'`
		p_tau_1=`tail -n 1 run-$u-$i/consumptions/energymrtk/0/all_tasks.txt | sed 's/.*P://;s/).*//'`  # period of a task
		n_instances=`echo "$hyp/$p_tau_1" | bc`
		missing_i_emrtk=`tail run-$u-$i/consumptions/energymrtk/0/log_emrtk_1_run.txt | grep "missing tasks:" | sed "s/.*missing tasks: //g;s/,.*//g"`
		missing_i_mrtk=`tail run-$u-$i/consumptions/mrtk/0/log_mrtk_1_run.txt | grep "missing tasks:" | sed "s/.*missing tasks: //g;s/,.*//g"`
                missing_i_grub_pa=`tail run-$u-$i/consumptions/grub_pa/0/log_mrtk_1_run.txt | grep "missing tasks:" | sed "s/.*missing tasks: //g;s/,.*//g"`
		echo "$perc_tot_dl_miss_emrtk + $missing_i_emrtk / $n_instances * 100"
		perc_tot_dl_miss_emrtk=`python -c "print round(float('$perc_tot_dl_miss_emrtk') + float('$missing_i_emrtk') / float('$n_instances') * 100, 4)"`
		perc_tot_dl_miss_mrtk=`python -c "print round(float('$perc_tot_dl_miss_mrtk') + float('$missing_i_mrtk') / float('$n_instances') * 100, 4)"`
		perc_tot_dl_miss_grub_pa=`python -c "print round(float('$perc_tot_dl_miss_grub_pa') + float('$missing_i_grub_pa') / float('$n_instances') * 100, 4)"`
		#exit 0
	done

	echo "$u & $tot_missing_tasks_10_exp_emrtk & $tot_missing_tasks_10_exp_mrtk &  $tot_missing_tasks_10_exp_grub_pa & $perc_tot_dl_miss_emrtk & $perc_tot_dl_miss_mrtk & $perc_tot_dl_miss_grub_pa \\\\" >> $fname
done

echo "output produced: $fname"
