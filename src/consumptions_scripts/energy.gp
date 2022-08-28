#!/usr/bin/gnuplot

set grid
set key out top center horizontal
set terminal pdfcairo color

set xlabel 'Time (ns)'
set ylabel 'W'

set output 'energy_gp_output_big_only_1exp_lines.pdf'

plot	'consumptions/grub_pa/0/powerBIG.txt_tickWatt_output.dat' u 1:2 t 'big, GRUB-PA' with lines, \
		'consumptions/energymrtk/0/powerBIG.txt_tickWatt_output.dat' u 1:2 t 'big, BL-CBS' with lines

set output 'energy_gp_output_little_only_1exp_lines.pdf'

plot	'consumptions/grub_pa/0/powerLITTLE.txt_tickWatt_output.dat' u 1:2 t 'LITTLE, GRUB-PA' with lines, \
		'consumptions/energymrtk/0/powerLITTLE.txt_tickWatt_output.dat' u 1:2 t 'LITTLE, BL-CBS' with lines
	


	
set output 'energy_gp_output_big_only_1exp_linespoints.pdf'

plot	'consumptions/grub_pa/0/powerBIG.txt_tickWatt_output.dat' u 1:(int($1) % 100000 == 0 ? $2 : 1/0) t 'big, GRUB-PA' with linespoints, \
		'consumptions/energymrtk/0/powerBIG.txt_tickWatt_output.dat' u 1:(int($1) % 100000 == 0 ? $2 : 1/0) t 'big, BL-CBS' with linespoints

set output 'energy_gp_output_little_only_1exp_linespoints.pdf'

plot	'consumptions/grub_pa/0/powerLITTLE.txt_tickWatt_output.dat' t 'LITTLE, GRUB-PA' with linespoints, \
		'consumptions/energymrtk/0/powerLITTLE.txt_tickWatt_output.dat' t 'LITTLE, BL-CBS' with linespoints
		
