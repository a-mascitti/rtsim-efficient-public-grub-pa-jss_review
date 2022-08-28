#!/usr/bin/gnuplot
  
# echo "$u $i $emrtk $mrtk $grubpa" >> scatter_hyperperiods.dat, ns


set grid
set key out top center
set terminal pdfcairo color

set xlabel 'Total per-core utilization'
set ylabel 'Hyperperiod (ms)'
set output 'scatter_hyperperp.pdf'

plot    'hyperperiods.dat' u 1:($3 / 1000000) t 'BL-CBS' with points lc rgb 'black'
