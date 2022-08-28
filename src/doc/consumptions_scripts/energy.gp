#!/usr/bin/gnuplot

set grid
set key top left

set terminal pdfcairo monochrome

set xlabel 'time (ms)'
set ylabel 'Energy'

set output 'energy_time_only_1.pdf'

plot	'energyB_MRTK_time.dat' t 'big G-EDF' with lines dashtype 2, 'energyL_MRTK_time.dat' t 'LITTLE G-EDF' with lines dashtype 1,\
		'energyB_EMRTK_time.dat' t 'big paper alg.' with lines lw 2 dashtype 2, 'energyL_EMRTK_time.dat' t 'LITTLE paper alg.' with lines dashtype 1
		
