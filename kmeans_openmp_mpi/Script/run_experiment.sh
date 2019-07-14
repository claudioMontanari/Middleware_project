#! /bin/bash
#
# The following script copy the input file in the fs
# of all the machines in the cluster (file ../slaves),
# compile the benchmark program and it runs it on the cluster.
# For a proper execution run the script from the
# kmeans_openmp_mpi directory.

run_exp(){
	cd /users/claudio/experiment
	mpirun -n 4 -mca btl ^openib --host $(cat ./slaves | tr '\n' ',' )  './benchmark' -i $INPUT_FILE -o $OUTPUT_FILE ${EXP_PARAMETERS[@]}
	if [ "$PLOT" = "true" ]; then 
		echo 'plotting resulting clustering'
		gnuplot -e "
set terminal 'pdfcairo';
set output './Pictures/${OUTPUT_FILE%.csv}.pdf';
set ylabel 'y';
set xlabel 'x';
set datafile separator ',';
plot '${INPUT_FILE}' using 1:2 with dot title 'datapoints', \
     '${OUTPUT_FILE}' using 1:2 title 'centroids';
"
	fi
}

EXPERIMENT=$1
INPUT_FILE=$2
OUTPUT_FILE=$3
read -a EXP_PARAMETERS <<< $4
#EXP_PARAMETERS='-c 100 -d 2 -t 4'
echo "parameters: ${EXP_PARAMETERS[@]}"

PLOT='true'
REMOTE_PATH='~/experiment/'
STARTING_PATH=$(pwd)

if [ "$#" -ne 4 ]; then
	echo 'Error, usage: run_experiment.sh experiment_name input_file output_file "input parameter list"'
	exit
fi

if [ ! -f $INPUT_FILE ] || [ ! -f $EXPERIMENT ]; then 
	echo 'Executable or input file not found!'
	exit 
fi

echo 'Copying the files into the FS of all machines'
parallel-ssh -i -h ./slaves mkdir -p $REMOTE_PATH'Data'
parallel-ssh -i -h ./slaves mkdir -p $REMOTE_PATH'Pictures'
for i in `seq 0 3`; do
	scp ./slaves node-$i:$REMOTE_PATH
	scp $EXPERIMENT node-$i:$REMOTE_PATH
	scp benchmark_mpi.h node-$i:$REMOTE_PATH
	scp $INPUT_FILE node-$i:$REMOTE_PATH'Data/'
done

echo 'Compiling the program'
# TODO: substitute with a proper call to the Makefile
parallel-ssh -i -h ./slaves "cd $REMOTE_PATH; mpicc -o benchmark $EXPERIMENT;"

echo ' =========================== Running the experiment =========================== '
run_exp

exit
