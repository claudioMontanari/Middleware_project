#! /bin/bash
#
# The following script copy the input file in the fs
# of all the machines in the cluster (file ../slaves),
# compile the benchmark program and it runs it on the cluster.
# For a proper execution run the script from the
# kmeans_openmp_mpi directory.

EXPERIMENT=$1
INPUT_FILE=$2
OUTPUT_FILE=$3
read -a EXP_PARAMETERS <<< $4

REMOTE_PATH='~/experiment'
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
parallel-ssh -i -h ./slaves mkdir -p $REMOTE_PATH'/Data'
for i in `seq 0 3`; do
	scp ./slaves node-$i:$REMOTE_PATH
	scp $EXPERIMENT node-$i:$REMOTE_PATH
	scp $INPUT_FILE node-$i:$REMOTE_PATH'/Data/'
done

echo 'Compiling the program'
parallel-ssh -i -h ./slaves 'cd $REMOTE_PATH; mpicc $EXPERIMENT -o experiment;'

echo 'Running the experiment'
cd $REMOTE_PATH
mpirun -np 1 --hostfile ./slaves ./experiment -o $OUTPUT_FILE $EXP_PARAMETERS
cp $OUTPUT_FILE $STARTING_PATH'/Data/' 

exit
