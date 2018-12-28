#! /bin/bash

# Parameters expected: 

PATH_DATAPOINTS=$1								# Path to datapoints file
PATH_OUTPUT=$2									# Path to centroids output
PATH_PLOT=$3									# Path to plot
NR_CENTROIDS=$4									# Number of centroids 

# Default values

echo start of the test

./benchmark -t 10000 -c $NR_CENTROIDS -d 2 -p $PATH_DATAPOINTS -o $PATH_OUTPUT -n 4

echo test finished, plotting the results.

gnuplot -e "set terminal 'pdfcairo';
set output '${PATH_PLOT}';
set ylabel 'y';
set xlabel 'x';
plot '${PATH_DATAPOINTS}' using 1:2 with dot title 'datapoints', \
	 '${PATH_OUTPUT}' using 1:2 title 'clusters';"

echo done.

evince $PATH_PLOT