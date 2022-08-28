#!/bin/bash


rm -f ./table_comparison.txt
rm -f ./table_comparison_single.txt

#utils="0.25 0.4 0.5 0.65"
utils="0.2 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.6 0.65 0.7"

exp_num=10
while [ "$1" != "" ]; do
    case $1 in
        --exp-num )           shift
                              exp_num=$1
                              ;;
        * )                   usage  # call usage()
                              exit 1
    esac   # end of case
    shift  # argument $2 becomes $1, and so on
done

for u in $utils; do
	# EMRTK
	f_avg_big_emrtk=0
	f_min_big_emrtk=0
	f_max_big_emrtk=0
	f_sd_big_emrtk=0
	f_avg_little_emrtk=0
	f_min_little_emrtk=0
	f_max_little_emrtk=0
	f_sd_little_emrtk=0

	total_energy_big_avg_emrtk=0
	total_energy_big_min_emrtk=0
	total_energy_big_max_emrtk=0
	total_energy_big_sd_emrtk=0
	total_energy_little_avg_emrtk=0
	total_energy_little_min_emrtk=0
	total_energy_little_max_emrtk=0
	total_energy_little_sd_emrtk=0

	# GRUBPA
	f_avg_big_grubpa=0
	f_min_big_grubpa=0
	f_max_big_grubpa=0
	f_sd_big_grubpa=0
	f_avg_little_grubpa=0
	f_min_little_grubpa=0
	f_max_little_grubpa=0
	f_sd_little_grubpa=0

	total_energy_big_avg_grubpa=0
	total_energy_big_min_grubpa=0
	total_energy_big_max_grubpa=0
	total_energy_big_sd_grubpa=0
	total_energy_little_avg_grubpa=0
	total_energy_little_min_grubpa=0
	total_energy_little_max_grubpa=0
	total_energy_little_sd_grubpa=0

	# EMRTK
	for((i=0;i<$exp_num;i++)); do
		python2 ./table_comparison.py run-$u-$i/freqBIG.dat
		temp=`cat table_comparison_py_output_temp.txt`  # expecting a single float value
		f_avg_big_emrtk=`echo "$f_avg_big_emrtk + $temp" | bc`
	done
	f_avg_big_emrtk=`echo "$f_avg_big_emrtk / 10" | bc`
	f_min_big_emrtk=`cat run-$u*/freqBIG.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --min`
	f_max_big_emrtk=`cat run-$u*/freqBIG.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --max`
	f_sd_big_emrtk=` cat run-$u*/freqBIG.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --dev | xargs printf "%.1f"`
	for((i=0;i<$exp_num;i++)); do
		python2 ./table_comparison.py run-$u-$i/freqLITTLE.dat
		temp=`cat table_comparison_py_output_temp.txt`  # expecting a single float value
		f_avg_little_emrtk=`echo "$f_avg_little_emrtk + $temp" | bc`
	done
	f_avg_little_emrtk=`echo "$f_avg_little_emrtk / $exp_num" | bc`
	f_min_little_emrtk=`cat run-$u*/freqLITTLE.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --min`
	f_max_little_emrtk=`cat run-$u*/freqLITTLE.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --max`
	f_sd_little_emrtk=` cat run-$u*/freqLITTLE.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --dev | xargs printf "%.1f"`

	# GRUBPA
	for((i=0;i<$exp_num;i++)); do
		python2 ./table_comparison.py run-$u-$i/freqBIG_grubpa.dat
		temp=`cat table_comparison_py_output_temp.txt`  # expecting a single float value
		f_avg_big_grubpa=`echo "$f_avg_big_grubpa + $temp" | bc`
	done
	f_avg_big_grubpa=`echo "$f_avg_big_grubpa / $exp_num" | bc`
	f_min_big_grubpa=`cat run-$u*/freqBIG_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --min`
	f_max_big_grubpa=`cat run-$u*/freqBIG_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --max`
	f_sd_big_grubpa=` cat run-$u*/freqBIG_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --dev | xargs printf "%.1f"`
	for((i=0;i<$exp_num;i++)); do
		python2 ./table_comparison.py run-$u-$i/freqLITTLE_grubpa.dat
		temp=`cat table_comparison_py_output_temp.txt`  # expecting a single float value
		f_avg_little_grubpa=`echo "$f_avg_little_grubpa + $temp" | bc`
	done
	f_avg_little_grubpa=`echo "$f_avg_little_grubpa / $exp_num" | bc`
	f_min_little_grubpa=`cat run-$u*/freqLITTLE_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --min`
	f_max_little_grubpa=`cat run-$u*/freqLITTLE_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --max`
	f_sd_little_grubpa=` cat run-$u*/freqLITTLE_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --dev | xargs printf "%.1f"`


	total_energy_big_avg_emrtk=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions big emrtk\"" | sed 's/"sum energy consumptions big emrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_big_min_emrtk=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions big emrtk\"" | sed 's/"sum energy consumptions big emrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --min | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_big_max_emrtk=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions big emrtk\"" | sed 's/"sum energy consumptions big emrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --max | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_big_sd_emrtk=` cat run-$u*/graph_py_output.json | grep "sum energy consumptions big emrtk\"" | sed 's/"sum energy consumptions big emrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --dev | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_avg_emrtk=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions little emrtk\"" | sed 's/"sum energy consumptions little emrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_min_emrtk=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions little emrtk\"" | sed 's/"sum energy consumptions little emrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --min | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_max_emrtk=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions little emrtk\"" | sed 's/"sum energy consumptions little emrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --max | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_sd_emrtk=` cat run-$u*/graph_py_output.json | grep "sum energy consumptions little emrtk\"" | sed 's/"sum energy consumptions little emrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --dev | xargs -I{} python -c "print int({} / 1000000)"`

	total_energy_big_avg_gedf=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions big mrtk" | sed 's/"sum energy consumptions big mrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_big_min_gedf=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions big mrtk" | sed 's/"sum energy consumptions big mrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --min | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_big_max_gedf=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions big mrtk" | sed 's/"sum energy consumptions big mrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --max | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_big_sd_gedf=` cat run-$u*/graph_py_output.json | grep "sum energy consumptions big mrtk" | sed 's/"sum energy consumptions big mrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --dev | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_avg_gedf=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions little mrtk" | sed 's/"sum energy consumptions little mrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_min_gedf=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions little mrtk" | sed 's/"sum energy consumptions little mrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --min | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_max_gedf=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions little mrtk" | sed 's/"sum energy consumptions little mrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --max | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_sd_gedf=` cat run-$u*/graph_py_output.json | grep "sum energy consumptions little mrtk" | sed 's/"sum energy consumptions little mrtk": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --dev | xargs -I{} python -c "print int({} / 1000000)"`

	total_energy_big_avg_grubpa=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions big grubpa" | sed 's/"sum energy consumptions big grubpa": //;s/,//' | awk '{$1=$1};1' | datastat -nh | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_big_min_grubpa=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions big grubpa" | sed 's/"sum energy consumptions big grubpa": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --min | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_big_max_grubpa=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions big grubpa" | sed 's/"sum energy consumptions big grubpa": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --max | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_big_sd_grubpa=` cat run-$u*/graph_py_output.json | grep "sum energy consumptions big grubpa" | sed 's/"sum energy consumptions big grubpa": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --dev | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_avg_grubpa=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions little grubpa" | sed 's/"sum energy consumptions little grubpa": //;s/,//' | awk '{$1=$1};1' | datastat -nh | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_min_grubpa=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions little grubpa" | sed 's/"sum energy consumptions little grubpa": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --min | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_max_grubpa=`cat run-$u*/graph_py_output.json | grep "sum energy consumptions little grubpa" | sed 's/"sum energy consumptions little grubpa": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --max | xargs -I{} python -c "print int({} / 1000000)"`
	total_energy_little_sd_grubpa=` cat run-$u*/graph_py_output.json | grep "sum energy consumptions little grubpa" | sed 's/"sum energy consumptions little grubpa": //;s/,//' | awk '{$1=$1};1' | datastat -nh -na --dev | xargs -I{} python -c "print int({} / 1000000)"`

	total_energy_avg_emrtk=`echo "$total_energy_big_avg_emrtk + $total_energy_little_avg_emrtk" | bc | xargs printf "%.0f"`  # big + little
	total_energy_min_emrtk=`echo "$total_energy_big_min_emrtk + $total_energy_little_min_emrtk" | bc | xargs printf "%.0f"`
	total_energy_max_emrtk=`echo "$total_energy_big_max_emrtk + $total_energy_little_max_emrtk" | bc | xargs printf "%.0f"`
	total_energy_sd_emrtk=` echo "$total_energy_big_sd_emrtk + $total_energy_little_sd_emrtk" | bc | xargs printf "%.0f"`

	total_energy_avg_gedf=`echo "$total_energy_big_avg_gedf + $total_energy_little_avg_gedf" | bc | xargs printf "%.0f"`
	total_energy_min_gedf=`echo "$total_energy_big_min_gedf + $total_energy_little_min_gedf" | bc | xargs printf "%.0f"`
	total_energy_max_gedf=`echo "$total_energy_big_max_gedf + $total_energy_little_max_gedf" | bc | xargs printf "%.0f"`
	total_energy_sd_gedf=` echo "$total_energy_big_sd_gedf + $total_energy_little_sd_gedf" | bc | xargs printf "%.0f"`

	total_energy_avg_grubpa=`echo "$total_energy_big_avg_grubpa + $total_energy_little_avg_grubpa" | bc | xargs printf "%.0f"`
	total_energy_min_grubpa=`echo "$total_energy_big_min_grubpa + $total_energy_little_min_grubpa" | bc | xargs printf "%.0f"`
	total_energy_max_grubpa=`echo "$total_energy_big_max_grubpa + $total_energy_little_max_grubpa" | bc | xargs printf "%.0f"`
	total_energy_sd_grubpa=` echo "$total_energy_big_sd_grubpa + $total_energy_little_sd_grubpa" | bc | xargs printf "%.0f"`

	# min max avg sd
	echo -e "\
	% 10 exp. W*ns / 1000.000 = W*ms = mJ
	\hline
	\hline
	& \multicolumn{12}{c|}{\$U_{core}\$ = $u} \\\\\\
	\hline
	\$f_{big}$ 		\t& $f_min_big_emrtk \t& $f_max_big_emrtk \t& $f_avg_big_emrtk \t& $f_sd_big_emrtk \t& 2000 \t& 2000 \t& 2000 \t& 2000 \t& $f_min_big_grubpa \t& $f_max_big_grubpa \t& $f_avg_big_grubpa \t& $f_sd_big_grubpa \\\\\\ 
	\$f_{LITTLE}$ 	\t& $f_min_little_emrtk \t& $f_max_little_emrtk \t& $f_avg_little_emrtk \t& $f_sd_little_emrtk \t& 1400 \t& 1400 \t& 1400 \t& 1400 \t& $f_min_little_grubpa \t& $f_max_little_grubpa \t& $f_avg_little_grubpa \t& $f_sd_little_grubpa \\\\\\ 
	\hline
	\$E_{big}$ 		\t& $total_energy_big_min_emrtk \t& $total_energy_big_max_emrtk \t& $total_energy_big_avg_emrtk \t& $total_energy_big_sd_emrtk  \t& $total_energy_big_min_gedf \t& $total_energy_big_max_gedf \t& $total_energy_big_avg_gedf \t& $total_energy_big_sd_gedf \t& $total_energy_big_min_grubpa \t& $total_energy_big_max_grubpa \t& $total_energy_big_avg_grubpa \t& $total_energy_big_sd_grubpa \\\\\\
	\$E_{little}$ 	\t& $total_energy_little_min_emrtk \t& $total_energy_little_max_emrtk \t& $total_energy_little_avg_emrtk \t& $total_energy_little_sd_emrtk  \t& $total_energy_little_min_gedf \t& $total_energy_little_max_gedf \t& $total_energy_little_avg_gedf \t& $total_energy_little_sd_gedf \t& $total_energy_little_min_grubpa \t& $total_energy_little_max_grubpa \t& $total_energy_little_avg_grubpa \t& $total_energy_little_sd_grubpa \\\\\\
	\$E_{tot}$ 		\t& $total_energy_min_emrtk \t& $total_energy_max_emrtk \t& $total_energy_avg_emrtk \t& $total_energy_sd_emrtk  \t& $total_energy_min_gedf \t& $total_energy_max_gedf \t& $total_energy_avg_gedf \t& $total_energy_sd_gedf \t& $total_energy_min_grubpa \t& $total_energy_max_grubpa \t& $total_energy_avg_grubpa \t& $total_energy_sd_grubpa \\\\\\
	" >> ./table_comparison.txt
done
echo "\hline" >> ./table_comparison.txt

#utils="0.25 0.4 0.5 0.65"
i=0
for u in $utils; do
	# EMRTK
	f_avg_big_emrtk=0
	f_min_big_emrtk=0
	f_max_big_emrtk=0
	f_sd_big_emrtk=0
	f_avg_little_emrtk=0
	f_min_little_emrtk=0
	f_max_little_emrtk=0
	f_sd_little_emrtk=0

	# GRUBPA
	f_avg_big_grubpa=0
	f_min_big_grubpa=0
	f_max_big_grubpa=0
	f_sd_big_grubpa=0
	f_avg_little_grubpa=0
	f_min_little_grubpa=0
	f_max_little_grubpa=0
	f_sd_little_grubpa=0

	# EMRTK
	python2 ./table_comparison.py run-$u-$i/freqBIG.dat
	f_avg_big_emrtk=`cat table_comparison_py_output_temp.txt`  # expecting a single float value
	f_min_big_emrtk=`cat run-$u-$i/freqBIG.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --min`
	f_max_big_emrtk=`cat run-$u-$i/freqBIG.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --max`
	f_sd_big_emrtk=` cat run-$u-$i/freqBIG.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --dev | xargs printf "%.1f"`

	python2 ./table_comparison.py run-$u-$i/freqLITTLE.dat
	f_avg_little_emrtk=`cat table_comparison_py_output_temp.txt`  # expecting a single float value
	f_min_little_emrtk=`cat run-$u-$i/freqLITTLE.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --min`
	f_max_little_emrtk=`cat run-$u-$i/freqLITTLE.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --max`
	f_sd_little_emrtk=` cat run-$u-$i/freqLITTLE.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --dev | xargs printf "%.1f"`

	# GRUBPA
	python2 ./table_comparison.py run-$u-$i/freqBIG_grubpa.dat
	f_avg_big_grubpa=`cat table_comparison_py_output_temp.txt`  # expecting a single float value
	f_min_big_grubpa=`cat run-$u-$i/freqBIG_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --min`
	f_max_big_grubpa=`cat run-$u-$i/freqBIG_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --max`
	f_sd_big_grubpa=` cat run-$u-$i/freqBIG_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --dev | xargs printf "%.1f"`

	python2 ./table_comparison.py run-$u-$i/freqLITTLE_grubpa.dat
	f_avg_little_grubpa=`cat table_comparison_py_output_temp.txt`  # expecting a single float value
	f_min_little_grubpa=`cat run-$u-$i/freqLITTLE_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --min`
	f_max_little_grubpa=`cat run-$u-$i/freqLITTLE_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --max`
	f_sd_little_grubpa=` cat run-$u-$i/freqLITTLE_grubpa.dat | cut -d ' ' -f2 | datastat -k 2 -nh -na --dev | xargs printf "%.1f"`


	total_energy_big_final_emrtk=`cat run-$u-$i/graph_py_output.json | grep "sum energy consumptions big emrtk\"" | sed 's/"sum energy consumptions big emrtk": //;s/,//' | xargs printf "%.0f"`
	total_energy_little_final_emrtk=`cat run-$u-$i/graph_py_output.json | grep "sum energy consumptions little emrtk\"" | sed 's/"sum energy consumptions little emrtk": //;s/,//' | xargs printf "%.0f"`
	total_energy_big_final_gedf=`cat run-$u-$i/graph_py_output.json | grep "sum energy consumptions big mrtk" | sed 's/"sum energy consumptions big mrtk": //;s/,//' | xargs printf "%.0f"`
	total_energy_little_final_gedf=`cat run-$u-$i/graph_py_output.json | grep "sum energy consumptions little mrtk" | sed 's/"sum energy consumptions little mrtk": //;s/,//' | xargs printf "%.0f"`
	total_energy_big_final_grubpa=`cat run-$u-$i/graph_py_output.json | grep "sum energy consumptions big grubpa" | sed 's/"sum energy consumptions big grubpa": //;s/,//' | xargs printf "%.0f"`
	total_energy_little_final_grubpa=`cat run-$u-$i/graph_py_output.json | grep "sum energy consumptions little grubpa" | sed 's/"sum energy consumptions little grubpa": //;s/,//' | xargs printf "%.0f"`


	total_energy_final_emrtk=`echo "($total_energy_big_final_emrtk + $total_energy_little_final_emrtk) / 1000000" | bc | xargs printf "%.0f"`  # big + little
	total_energy_final_gedf=`echo "($total_energy_big_final_gedf + $total_energy_little_final_gedf) / 1000000" | bc | xargs printf "%.0f"`
	total_energy_final_grubpa=`echo "($total_energy_big_final_grubpa + $total_energy_little_final_grubpa) / 1000000" | bc | xargs printf "%.0f"`

	# min max avg sd
	echo -e "\
	% 1 exp (exp number 0). W*ns / 1000.000 = W*ms = mJ
	\hline
	\hline
	& \multicolumn{12}{c|}{\$U_{core}\$ = $u} \\\\\\
	\hline
	\$f_{big}$ 		\t& $f_min_big_emrtk \t& $f_max_big_emrtk \t& $f_avg_big_emrtk \t& $f_sd_big_emrtk \t& 2000 \t& 2000 \t& 2000 \t& 2000 \t& $f_min_big_grubpa \t& $f_max_big_grubpa \t& $f_avg_big_grubpa \t& $f_sd_big_grubpa \\\\\\ 
	\$f_{LITTLE}$ 	\t& $f_min_little_emrtk \t& $f_max_little_emrtk \t& $f_avg_little_emrtk \t& $f_sd_little_emrtk \t& 1400 \t& 1400 \t& 1400 \t& 1400 \t& $f_min_little_grubpa \t& $f_max_little_grubpa \t& $f_avg_little_grubpa \t& $f_sd_little_grubpa \\\\\\ 
	\hline
	\$E_{tot}$   & \multicolumn{4}{|c|}{$total_energy_final_emrtk} & \multicolumn{4}{|c|}{$total_energy_final_gedf} & \multicolumn{4}{|c|}{$total_energy_final_grubpa} \\\\\\ 
	" >> ./table_comparison_single.txt
done

echo "\hline" >> ./table_comparison_single.txt
rm table_comparison_py_output_temp.txt

echo "results of $0 saved into ./table_comparison.txt and ./table_comparison_single.txt"
