#!/usr/bin/gnuplot

set grid  # opzionale
set key out top center horizontal
set key autotitle columnheader
set terminal pdfcairo color
#set title 'My title'

set xlabel 'Total per-core utilization'
set ylabel 'Average energy cons. (mJ)'
set xrange [0.15:0.75]
set xtics 0.05

set output 'errorbar_energy.pdf


plot 'errorbar_energy.dat' using ($1-0.01):(stringcolumn(4) eq "BL-CBS" ? ($2/1000000) : 1/0):($3/1000000) with yerrorbars title 'BL-CBS' lc rgb "forest-green", \
     'errorbar_energy.dat' using ($1+0.01):(stringcolumn(4) eq "GRUBPA" ? ($2/1000000) : 1/0):($3/1000000) with yerrorbars title 'GRUB-PA' lc rgb "dark-magenta"
