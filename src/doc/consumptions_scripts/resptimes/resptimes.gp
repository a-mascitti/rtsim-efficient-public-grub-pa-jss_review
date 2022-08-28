#!/usr/bin/gnuplot

set grid
set key bottom right

set terminal pdfcairo color

set xlabel 'Response time / period'
set ylabel 'Experimental global CDF'

n=system("grep -c '' resptimes_emrtk_all.dat")
n_mrtk=system("grep -c '' resptimes_mrtk_all.dat")

set output 'resptimes_emrtk_and_mrtk_globalCDF.pdf'

plot 'resptimes_emrtk_all.dat' u 1:($0/n) t 'paper alg.' with lines, 'resptimes_mrtk_all.dat' u 1:($0/n_mrtk) t 'G-EDF' with lines
