#!/usr/bin/gnuplot

set grid
set key bottom right

set terminal pdfcairo color

set xlabel 'Response time / period'
set ylabel 'Experimental CDF'

n=system("grep -c '' consumptions/energymrtk/0/resptimes_emrtk.dat")
n_mrtk=system("grep -c '' consumptions/mrtk/0/resptimes_mrtk.dat")
n_grubpa=system("grep -c '' consumptions/grub_pa/0/resptimes_grubpa.dat")
n_bf=system("grep -c '' consumptions/energymrtk_bf/0/resptimes_emrtk_bf.dat")
n_ff=system("grep -c '' consumptions/energymrtk_ff/0/resptimes_emrtk_ff.dat")

set output 'resptimes.pdf'

plot 	'consumptions/energymrtk/0/resptimes_emrtk_all.dat' u 1:($0/n) t 'BL-CBS' with lines, \
	'consumptions/mrtk/0/resptimes_mrtk_all.dat' u 1:($0/n_mrtk) t 'G-EDF' with lines, \
	'consumptions/grub_pa/0/resptimes_grubpa_all.dat' u 1:($0/n_grubpa) t 'GRUB-PA' with lines, \
	'consumptions/energymrtk_bf/0/resptimes_emrtk_bf_all.dat' u 1:($0/n_bf) t 'EDF-BF' with lines, \
	'consumptions/energymrtk_ff/0/resptimes_emrtk_ff_all.dat' u 1:($0/n_ff) t 'EDF-FF' with lines
