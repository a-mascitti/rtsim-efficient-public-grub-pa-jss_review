#!/usr/bin/gnuplot
  
# echo "$u BL-CBS $energy_emrtk $i" >> scatter.dat, W*ns


set grid
set key outside top center horizontal
set terminal pdfcairo color
set key autotitle columnheader

set xlabel 'Total utilization'
set ylabel 'Total energy cons. (mJ)'
set output 'scatter_energy.pdf'

plot    'scatter_energy.dat' u 1:(stringcolumn(2) eq "BL-CBS" ? $3 / 1000000 : 1/0) t 'BL-CBS' with points lc rgb 'green', \
      'scatter_energy.dat' u 1:(stringcolumn(2) eq "GRUBPA" ? $3 / 1000000 : 1/0) t 'GRUB-PA' with points lc rgb 'black'


set output 'scatter_energy_ratio.pdf'
set ylabel 'E_{tot, GRUB-PA} / E_{tot, BL-CBS}'
plot 	'scatter_energy_ratio.dat' u 1:($2 / 1000000) t 'One experiment'

set output 'scatter_energy_ratio_avg.pdf'
set ylabel 'Energy ratio'
#set xrange [0.15:0.75]
set xrange [1.2:6]
set yrange [0.8:7.0]
#set xtics autofreq 0.05
set xtics autofreq 0.4
set ytics (0.8, 1, 1.2, 1.5, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0)
set logscale y
plot 	'scatter_energy_ratio_avg.dat' u ($1*8):2:3:4 t 'E_{tot, GRUB-PA}/E_{tot, BL-CBS}' lc rgb 'dark-green' lw 2 dt 1 with yerrorbars, \
	'scatter_energy_ratio_avg.dat' u ($1*8):5:6:7 t 'E_{tot, G-EDF}/E_{tot, BL-CBS}' lc rgb 'black' lw 2 dt 2 with yerrorbars, \
        'scatter_energy_ratio_avg.dat' u ($1*8):8:9:10 t 'E_{tot, EDF-BF}/E_{tot, BL-CBS}' lc rgb 'red' lw 2 dt 3 with yerrorbars, \
        'scatter_energy_ratio_avg.dat' u ($1*8):11:12:13 t 'E_{tot, EDF-FF}/E_{tot, BL-CBS}' lc rgb 'blue' lw 2 dt 4 with yerrorbars