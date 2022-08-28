## How I produced the 2 PDFs

```
cd rtsim/src/consumption/
# process 1 experiment, all files below must be in the same folder
grep ended trace25_mrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > resptimes_mrtk.dat
grep ended trace25_emrtk.txt | sed -e 's/\[Time://; s/].*arrival was//; s/, its period was//' | awk '{print $1, $1-$2, ($1-$2)/$3;}' > resptimes_emrtk.dat
./draw-resptimes.gp
gv resptimes_emrtk_and_mrtk.pdf
```