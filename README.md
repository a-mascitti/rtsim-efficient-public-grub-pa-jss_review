# How to use

```
git clone git@gitlab.retis.santannapisa.it:a.mascitti/rtsim-efficient-public-grub-pa 
cd rtsim-efficient-public-grub-pa/src/
git checkout jss_review
make -j4
./reviewer_launch_me.sh  # and answer 'y' to generate and simulate random task-sets
```

# Replicating paper graphs

This section explains how to replicate the graphs in the Simulation section of the JSS paper:

http://retis.sssup.it/~tommaso/papers/jss20.php

```
git clone git@gitlab.retis.santannapisa.it:a.mascitti/rtsim-efficient-public-grub-pa 
cd rtsim-efficient-public-grub-pa/src/
git checkout jss_review
make -j4
wget  --no-check-certificate "https://owncloud.retis.santannapisa.it/index.php/s/k4aaeuYzSA1HD0l/download" -O runs_jss_review_edfff_edfbf.tar.gz
tar -xzf runs_jss_review_edfff_edfbf.tar.gz
./reviewer_launch_me.sh  # and answer 'n' to skip the generation of new task-sets; takes 15 minutes on Intel i5-8th
evince energies_utilizations.pdf  # Figure 3 left
evince scatter_energy_ratio_avg.pdf  # Figure 3 right
evince frequencies_utilizations_big.pdf  # Figure 4 left
evince frequencies_utilizations_big.pdf  # Figure 4 right
evince run-0.25-0/pow_ticks_little.pdf  # Figure 5a
evince run-0.25-0/pow_ticks_big.pdf  # Figure 5b
evince run-0.5-5/pow_ticks_little.pdf  # Figure 5c
evince run-0.5-5/pow_ticks_big.pdf  # Figure 5d
evince run-0.25/resptimes_emrtk_and_mrtk_and_grubpa_globalCDF.pdf  # Figure 6
```

Other figures (Figure 1 and 2) are obtained from previous papers (for more information, refer to Prof. Tommaso Cucinotta).