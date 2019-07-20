#! /bin/bash
#
# The following script copies the benchmark surce code and
# the data input file in the fs of all the machines in the cluster (file ../slaves);
# it compiles the benchmark program and it runs it on the cluster.
# For a proper execution run the script from the
# kmeans_openmp_mpi directory.

run_exp(){
    	EXP_PATH=$(readlink -f ~/experiment/slaves)
	cd ${EXP_PATH%/*}
	mpirun -n ${#NODES[@]} -mca btl ^openib --host $(cat ./slaves | tr '\n' ',' )  './kmeans' -i $INPUT_FILE -o $OUTPUT_FILE ${EXP_PARAMETERS[@]}

	if [ "$PLOT" = "plot" ]; then 
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

INPUT_FILE=$1
OUTPUT_FILE=$2
read -a EXP_PARAMETERS <<< $3
PLOT=$4
echo "parameters: ${EXP_PARAMETERS[@]}"

REMOTE_PATH='~/experiment/'

if [ "$#" -ne 4 ] && [ "$#" -ne 3 ] ; then
	echo 'Error, usage: run_experiment.sh input_file output_file "input parameter list" [plot]'
	exit
fi

if [ ! -f $INPUT_FILE ] || [ ! -f ./benchmark_mpi.c ]; then 
	echo 'Executable or input file not found!'
	exit 
fi

echo 'Copying the files into the FS of all machines'
parallel-ssh -i -h ./slaves mkdir -p $REMOTE_PATH'Data'
parallel-ssh -i -h ./slaves mkdir -p $REMOTE_PATH'Pictures'

readarray NODES < ./slaves
FILES=('slaves' 'benchmark_mpi.h' 'benchmark_mpi.c' 'Makefile')

for n in ${NODES[@]}; do
	for f in ${FILES[@]}; do
	    scp $f $n:$REMOTE_PATH >/dev/null
	done	
	scp $INPUT_FILE $n:$REMOTE_PATH'Data/' >/dev/null
done

echo 'Compiling the program'
#parallel-ssh -i -h ./slaves "cd $REMOTE_PATH; mpicc -o kmeans $EXPERIMENT -fopenmp;"
#parallel-ssh -i -h ./slaves "cd $REMOTE_PATH; mpicc -o kmeans $EXPERIMENT -fopenmp -DUSE_OMP;"
parallel-ssh -i -h ./slaves "cd $REMOTE_PATH; make clean kmeans;" >/dev/null 2>&1

echo ' =========================== Running the experiment =========================== '
run_exp

exit
