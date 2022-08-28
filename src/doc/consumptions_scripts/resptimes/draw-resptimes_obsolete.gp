#!/usr/bin/gnuplot

set terminal pdfcairo color

# set graph ration (reduce width)
set size ratio 0.5

# remove white margins
set lmargin at screen 0.1
set rmargin at screen 0.99
set bmargin at screen 0.13
set tmargin at screen 1

n_emrtk=system("grep -c '' resptimes_emrtk.dat")
n_mrtk=system("grep -c '' resptimes_mrtk.dat")
set grid

set xlabel 'response time / period'
set ylabel 'Experimental CDF'


set output 'resptimes_emrtk.pdf'
plot '< cat resptimes_emrtk.dat | LC_ALL=C sort -k3' u 3:($0/n_emrtk) with lines linewidth 3 title 'Response time CDF'

set output 'resptimes_emrtk_and_mrtk.pdf'
plot '< cat resptimes_emrtk.dat | LC_ALL=C sort -k3' u 3:($0/n_emrtk) with lines title 'EnergyMRTK', \
     '< cat resptimes_mrtk.dat | LC_ALL=C sort -k3' u 3:($0/n_mrtk) with lines title 'GEDF'
