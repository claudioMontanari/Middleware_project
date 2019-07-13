#! /bin/bash
#
# The following script copy the input file in the fs
# of all the machines in the cluster (file ../slaves),
# compile the benchmark program and it runs it on the cluster.
# For a proper execution run the script from the
# kmeans_openmp_mpi directory.

run_exp(){
    cd /users/claudio/experiment
    mpirun -n 4 -mca btl ^openib --host $(cat ./slaves | tr '\n' ',' )  './benchmark' -o $OUTPUT_FILE $EXP_PARAMETERS
}

EXPERIMENT=$1
INPUT_FILE=$2
OUTPUT_FILE=$3
read -a EXP_PARAMETERS <<< $4
EXP_PARAMETERS='-c 3 -d 3 -t 4'
echo "parameters: $EXP_PARAMETERS"


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
for i in `seq 0 3`; do
	scp ./slaves node-$i:$REMOTE_PATH
	scp $EXPERIMENT node-$i:$REMOTE_PATH
	scp benchmark_mpi.h node-$i:$REMOTE_PATH
	scp $INPUT_FILE node-$i:$REMOTE_PATH'Data/'
done

echo 'Compiling the program'
parallel-ssh -i -h ./slaves "cd $REMOTE_PATH; mpicc -o benchmark $EXPERIMENT -DDEBUG;"

echo 'Running the experiment'
run_exp

exit
