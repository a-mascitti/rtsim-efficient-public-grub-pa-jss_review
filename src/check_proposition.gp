#!/usr/bin/gnuplot
  
# echo "$u $perc_theor1" >> aggregate_check_propositions.dat

set grid
set key out top center
set terminal pdfcairo color
set key autotitle columnheader
set xrange [0.15:0.75]
# set xtics 0.05

set xlabel 'Total per-core utilization'
set ylabel 'Percentage accepted tasksets'
set output 'check_proposition.pdf'

plot    'aggregate_check_propositions.dat' u 1:2 t 'BL-CBS' with points ls 2 
