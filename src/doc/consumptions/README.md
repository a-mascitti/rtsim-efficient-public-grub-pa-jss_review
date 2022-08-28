Basically this folder was only emrtk/ and mrtk/. They contain the data of the experiments (for each one, consumption on cores t=0...t=tasks hyperp and log file).
The script ./graph.py allows to make an average of consumption and graph them.

Then the other some graphs were needed and so the other folders were creared. Graphs show average of the 10 experiments. They extract data from logs.
- core_busy/	- graph of x=time, y=how many cores are busy
- freq/ 		- graph of x=time, y=used frequency
- resptimes/ 	- CDF graph of response times


Ewili2019 graphs can be obtained with ./graph_only_1.py, ./core_busy/core_busy.py, ./resptimes/resptimes_only_1.py

EuroSys2020 graphs can be obtained with the script src/launch_me.sh in SAC2020_alg_efficient, commit id 3677d337eb137362ece333c4c2d563532a9a8d88


How to use the scripts?
chmod +x ./launch_me.sh && ./launch_me.sh
