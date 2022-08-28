#!/usr/bin/gnuplot

set grid
set key autotitle columnheader
set key bottom right
set style fill solid border rgb "black"
set terminal pdfcairo color  # vogliamo un PDF in colore

set xlabel 'Island W'
set ylabel "Experimental CDF"
set yrange[0:1]

hyperp=system("head -n 325 consumptions/energymrtk/0/log*.txt | grep simulation_steps | sed 's/simulation_steps=//'")
print(hyperp)

set output 'pow_ticks_big.pdf'

plot  'pow_ticks_big_emrtk_output.dat'     using 1:4 t 'big, BL-CBS' with lines linecolor rgb 'dark-green' dt 2 , \
      'pow_ticks_big_grubpa_output.dat'    using 1:4 t 'big, GRUB-PA' with lines linecolor rgb 'black' dt 1, \
      'pow_ticks_big_emrtk_bf_output.dat'    using 1:4 t 'big, EDF-BF' with lines linecolor rgb 'red' dt 4, \
      'pow_ticks_big_emrtk_ff_output.dat'    using 1:4 t 'big, EDF-FF' with lines linecolor rgb 'blue' dt 5

set output 'pow_ticks_little.pdf'

plot  'pow_ticks_little_emrtk_output.dat'  using 1:4 t 'LITTLE, BL-CBS' with lines linecolor rgb 'dark-green' dt 2, \
      'pow_ticks_little_grubpa_output.dat' using 1:4 t 'LITTLE, GRUB-PA' with lines linecolor rgb 'black' dt 1, \
      'pow_ticks_little_emrtk_bf_output.dat' using 1:4 t 'LITTLE, EDF-BF' with lines linecolor rgb 'red' dt 4, \
      'pow_ticks_little_emrtk_ff_output.dat' using 1:4 t 'LITTLE, EDF-FF' with lines linecolor rgb 'blue' dt 5
