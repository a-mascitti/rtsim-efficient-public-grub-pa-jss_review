#!/usr/bin/gnuplot

set grid
set key top right

set terminal pdfcairo color

set xlabel 'time (ms)'
set ylabel 'freq. (MHz)'

set xrange [0:100000]

set output 'results_freq_over_time_exp0.pdf'

plot 'freqBIG.dat' t 'paper alg. big' with lines, 'freqLITTLE.dat' t 'paper alg. LITTLE' with lines