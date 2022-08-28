#!/bin/bash

echo "resptimes.sh has pwd `pwd`"

# take the resptimes of each experiment and append them into a common list (you get a list of resptimes / periods)
cat consumptions/energymrtk/0/resptimes_emrtk.dat | cut -d ' ' -f3 | LC_ALL=C sort -n | less > consumptions/energymrtk/0/resptimes_emrtk_all.dat

# same for BF
cat consumptions/energymrtk_bf/0/resptimes_emrtk_bf.dat | cut -d ' ' -f3 | LC_ALL=C sort -n | less > consumptions/energymrtk_bf/0/resptimes_emrtk_bf_all.dat

# same for FF
cat consumptions/energymrtk_ff/0/resptimes_emrtk_ff.dat | cut -d ' ' -f3 | LC_ALL=C sort -n | less > consumptions/energymrtk_ff/0/resptimes_emrtk_ff_all.dat

# same for mrtk experiments
cat consumptions/mrtk/0/resptimes_mrtk.dat | cut -d ' ' -f3 | LC_ALL=C sort -n | less > consumptions/mrtk/0/resptimes_mrtk_all.dat

# take the resptimes of each experiment and append them into a common list (you get a list of resptimes / periods)
cat consumptions/grub_pa/0/resptimes_grubpa.dat | cut -d ' ' -f3 | LC_ALL=C sort -n | less > consumptions/grub_pa/0/resptimes_grubpa_all.dat

# plot the 2 resulting lists (only resptimes / periods is considered for y axis)
./resptimes.gp
