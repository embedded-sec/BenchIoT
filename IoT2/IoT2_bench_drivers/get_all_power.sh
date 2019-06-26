#! /bin/bash
# Takes as input directory in which ever CSV file will be run through
# the compute_power.py is run

 echo "Benchmark, Max, Mean, Min, #Iters" > power_results.csv
for csv in $1/*.csv
do
    python3 compute_power_from_csv.py -c ${csv} >> power_results.csv
done 

