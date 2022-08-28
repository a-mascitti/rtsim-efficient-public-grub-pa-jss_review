#!/usr/bin/gnuplot

set grid  
set key outside top center horizontal
set key autotitle columnheader
set terminal pdfcairo monochrome

set xlabel 'Total per-core utilization'
set ylabel 'Average number of migrations'

set output 'migrations.pdf'

plot 'migrations.dat' u 1:2 t 'BL-CBS' with linespoints, \
     ''               u 1:4 t 'EDF-FF' with linespoints, \
     ''               u 1:5 t 'GRUB-PA' with linespoints