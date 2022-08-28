#!/usr/bin/gnuplot

set grid
set key below

set terminal pdfcairo monochrome

set xlabel 'time (ms)'
set ylabel 'W'

set xrange [0:100000]

set output 'results_big_exp0.pdf'

plot 'sumB_eachTick_EMRTK_exp0.dat' t 'big, BL-CBS' with lines dashtype 1, 'sumB_eachTick_MRTK_exp0.dat' t 'big, G-EDF' with lines


set output 'results_LITTLE_exp0.pdf'

plot 'sumL_eachTick_EMRTK_exp0.dat' t 'LITTLE, BL-CBS.' with lines dashtype 1, 'sumL_eachTick_MRTK_exp0.dat' t 'LITTLE, G-EDF' with lines
