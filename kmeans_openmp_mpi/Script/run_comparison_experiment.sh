#! /bin/bash
#
# The following script generates an input dataset
# and then runs the benchmark_mpi.c program over such dataset
# varying the numbers of threads and the number of machines used.
# Finally, a comparison plot of different execution times is generated

run_experiment(){
    EXP_PATH=$(readlink -f ~/experiment/slaves)
    cd ${EXP_PATH%/*}
    rm $OUTPUT
    for m in ${MACHINE_NR[@]}; do
	echo "Testing with $m nodes"
	
	printf "\n\n \"$m\" nodes\n" >> $OUTPUT
	for t in ${THREAD_NR[@]}; do
	    mpirun -n $m -mca btl ^openib --host $(cat ./slaves | tr '\n' ',' )  './kmeans' -i "Data/big_input_$CLUSTER_SIZE.csv" -o ./Data/output.csv -c 6 -d 2 -n $t | awk '/Thread_nr/ {printf "%d ", $2} /Duration/ {print $2}' >> $OUTPUT

	    sleep 2s	    
	done
    done
    

}

OPTION=$1

CLUSTER_SIZE=5000
readarray NODES < ./slaves
FILES=('slaves' 'benchmark_mpi.c' 'benchmark_mpi.h' 'Makefile' "Data/big_input_$CLUSTER_SIZE.csv")
THREAD_NR=(1 2 4 8 16 40)
MACHINE_NR=(1 2 4)

REMOTE_PATH='~/experiment/'
OUTPUT='Data/comparison.txt'

if [ "$#" -ne 1 ]; then
    echo 'Error, usage: run_comparison_exp.sh (all|plot)'
    exit
fi

if [ "$OPTION" = "all" ]; then
    # Generate input file - it will always have 6 centroids in a 2D space
    python3 ./Script/generate_dataset.py 1000 "Data/big_input_$CLUSTER_SIZE.csv"
    
    # Copy files on all machines & compile
    parallel-ssh -i -h ./slaves mkdir -p $REMOTE_PATH
    for m in ${NODES[@]}; do
	for f in ${FILES[@]}; do
	    scp $f $n:$REMOTE_PATH >/dev/null
	done
    done
    parallel-ssh -i -h ./slaves "cd $REMOTE_PATH; make clean kmeans;" > /dev/null 2>&1

    #  Run the experiments
    run_experiments

    # Copy the results into the Data directory 
    cp "$REMOTE_PATH/$OUTPUT" ./Data/
    
fi


# Plot the results found
gnuplot -e "
set terminal 'pdfcairo';
set output './Pictures/comparison.pdf';
set xlabel '# Threads';
set ylabel 'Duration (ms)';
plot for [i=0:*] '$OUTPUT' index i using 1:2 with linespoints title coloumhead(1) lw 2 ps .75 lc i+1; 
"

exit

