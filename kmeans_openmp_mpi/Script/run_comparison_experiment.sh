#! /bin/bash
#
# The following script generates an input dataset
# and then runs the benchmark_mpi.c program over such dataset
# varying the numbers of threads and the number of machines used.
# Finally, a comparison plot of different execution times is generated

start_dstat(){
    parallel-ssh -i -h ./slaves "rm dstat_${PLOT_NAME%.png}.csv; dstat --output dstat_${PLOT_NAME%.png}.csv  &>/dev/null &"
}

terminate_dstat(){
    parallel-ssh -i -h ./slaves "ps aux | grep -i 'dstat' | grep -Eiv 'ssh|grep' | awk -F' ' '{print \$2}' | xargs kill -9"
}

run_experiments(){
    EXP_PATH=$(readlink -f ~/experiment/slaves)
    cd ${EXP_PATH%/*}
    rm $OUTPUT
    touch $OUTPUT

    start_dstat
    for m in ${MACHINE_NR[@]}; do
	echo "Testing with $m nodes"
	
	printf "\n\n \"$m machines\"\n" >> $OUTPUT
	for t in ${THREAD_NR[@]}; do
	    mpirun -n $m -mca btl ^openib --host $(cat ./slaves | tr '\n' ',' )  './kmeans' -i "./big_input_$CLUSTER_SIZE.csv" -o ./Data/output.csv -c 6 -d 2 -n $t | awk '/Thread_nr/ {printf "%d ", $2} /Duration/ {print $2}' >> $OUTPUT

	    sleep 2s	    
	done
    done
    terminate_dstat
}

OPTION=$1
CLUSTER_SIZE=${2:-2000}
PLOT_NAME=${3:-'comparison.png'}

readarray NODES < ./slaves
FILES=('slaves' 'benchmark_mpi.c' 'benchmark_mpi.h' 'Makefile' "Data/big_input_$CLUSTER_SIZE.csv")
THREAD_NR=(1 2 4 8 16 40)
MACHINE_NR=(1 2 4)

STARTING_PATH=$(pwd)
REMOTE_PATH='~/experiment/'
OUTPUT='Data/comparison.txt'

if [ "$#" -lt 1 ] || [ "$#" -gt 3 ] ; then
    echo 'Error, usage: run_comparison_exp.sh (all|plot) cluster_size plot_name'
    exit
fi

if [ "$OPTION" = "all" ]; then
    # Generate input file - it will always have 6 centroids in a 2D space
    python3 ./Script/generate_dataset.py $CLUSTER_SIZE "./Data/big_input_$CLUSTER_SIZE.csv"
    
    # Copy files on all machines & compile
    parallel-ssh -i -h ./slaves mkdir -p "${REMOTE_PATH}Data"

    for m in ${NODES[@]}; do
	for f in ${FILES[@]}; do
	    scp $f $m:$REMOTE_PATH >/dev/null
	done
    done
    parallel-ssh -i -h ./slaves "cd $REMOTE_PATH; make clean kmeans;" > /dev/null 2>&1

    #  Run the experiments
    run_experiments

    # Copy the results into the Data directory
    SRC_DATA=$(readlink -f ~/experiment/slaves)
    #echo "${SRC_DATA%/*}/${OUTPUT} to ${STARTING_PATH}/${OUTPUT}"
    cp "${SRC_DATA%/*}/${OUTPUT}" "${STARTING_PATH}/${OUTPUT}"

fi

# Plot the results found
gnuplot -e "
set terminal 'pngcairo';
set output './Pictures/${PLOT_NAME}';
set xlabel '# Threads per machine';
set ylabel 'Duration (ms)';
set xrange [0:46];
plot for [i=0:*] '$OUTPUT' index i using 1:2 with linespoints title columnhead(1) lw 2 ps .75 lc i+1; 
"

exit

