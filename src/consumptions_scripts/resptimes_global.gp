#!/usr/bin/gnuplot

set grid
set key bottom right

set terminal pdfcairo color #monochrome


set xlabel 'Response time / period'
set ylabel 'Experimental global CDF'

n=system("grep -c '' resptimes_emrtk_all.dat")
n_mrtk=system("grep -c '' resptimes_mrtk_all.dat")
n_grubpa=system("grep -c '' resptimes_grubpa_all.dat")
n_bf=system("grep -c '' resptimes_emrtk_bf_all.dat")
n_ff=system("grep -c '' resptimes_emrtk_ff_all.dat")

set output 'resptimes_emrtk_and_mrtk_and_grubpa_globalCDF.pdf'

plot 	'resptimes_emrtk_all.dat' u 1:($0/n) t 'BL-CBS' with lines lw 3 lt 2, \
		'resptimes_mrtk_all.dat' u 1:($0/n_mrtk) t 'G-EDF' with lines dashtype 2 lw 2 lt 1, \
		'resptimes_grubpa_all.dat' u 1:($0/n_grubpa) t 'GRUB-PA' with lines dashtype 3 lw 2 lt 3,\
		'resptimes_emrtk_bf_all.dat' u 1:($0/n_bf) t 'EDF-BF' with lines dt 1 lt 8, \
		'resptimes_emrtk_ff_all.dat' u 1:($0/n_ff) t 'EDF-FF' with lines dt 2 lt 7