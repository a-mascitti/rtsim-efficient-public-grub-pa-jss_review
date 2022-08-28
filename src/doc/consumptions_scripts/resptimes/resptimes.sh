#!/bin/bash

echo "resptimes.sh has pwd `pwd`"

# take the resptimes of each experiment and append them into a common list (you get a list of resptimes / periods)
cd ../consumptions/energymrtk/
cat */resptimes_emrtk.dat | cut -d ' ' -f3 | LC_ALL=C sort -n | less > ../../resptimes/resptimes_emrtk_all.dat

# same for mrtk experiments
cd ../../consumptions/mrtk/
cat */resptimes_mrtk.dat | cut -d ' ' -f3 | LC_ALL=C sort -n | less > ../../resptimes/resptimes_mrtk_all.dat

# plot the 2 resulting lists (only resptimes / periods is considered for y axis)
cd ../../resptimes/
./resptimes.gp