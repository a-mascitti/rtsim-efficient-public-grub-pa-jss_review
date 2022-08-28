#!/usr/bin/gnuplot

set grid
set key out top center

set terminal pdfcairo color

set xlabel 'time (ns)'
set ylabel 'freq. (MHz)'

set xrange [0:1000000]

set output 'results_freq_over_time_exp0.pdf'

plot 	'freqBIG.dat' t 'paper alg. big' with points, \
	'freqLITTLE.dat' t 'paper alg. LITTLE' with points

# ------------------------------------

set grid
set key out top center
unset xrange
set terminal pdfcairo color
set style data histograms

set xlabel 'Frequency (MHz)'
set ylabel 'Total time of the frequency (ns)'
set xtics font ", 8" rotate by 90 offset 0,-0.8  # -0.8 le sposta in basso

set output 'freq_one_exp_histog_emrtk_grubpa.pdf'

# MHz big_amount_tick_total little_amount_tick_total

plot	'consumptions/energymrtk/0/freq_histog.dat' u 2:xtic(1) t 'big, BL-CBS', \
	'consumptions/grub_pa/0/freq_histog.dat' u 2:xtic(1) t 'big, GRUB-PA', \
	'consumptions/energymrtk/0/freq_histog.dat' u 3:xtic(1) t 'LITTLE, BL-CBS', \
	'consumptions/grub_pa/0/freq_histog.dat' u 3:xtic(1) t 'LITTLE, GRUB-PA'

