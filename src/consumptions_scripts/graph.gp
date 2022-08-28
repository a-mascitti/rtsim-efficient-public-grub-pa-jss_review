#!/usr/bin/gnuplot

set grid
set key below

set terminal pdfcairo monochrome

set xlabel 'time (ns)'
set ylabel 'W'

set xrange [0:100000000]

set output 'results_big_exp0.pdf'

plot 	'sumB_eachTick_EMRTK_exp0.dat' t 'big, BL-CBS' with lines dashtype 1, \
	'sumB_eachTick_GRUBPA_exp0.dat' t 'big, GRUB-PA' with lines dashtype 2


set output 'results_LITTLE_exp0.pdf'

plot 	'sumL_eachTick_EMRTK_exp0.dat' t 'LITTLE, BL-CBS.' with lines dashtype 1, \
	'sumL_eachTick_GRUBPA_exp0.dat' t 'LITTLE, GRUB-PA' with lines dashtype 2
