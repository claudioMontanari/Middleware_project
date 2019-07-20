#! /bin/bash
#
# The following script generates an input dataset
# and then runs the benchmark_mpi.c program over such dataset
# varying the numbers of threads and the number of machines used.
# Finally, a comparison plot of different execution times is generated


OPTION=$1

SIZE=5000
readarray NODES < ./slaves
FILES=('slaves' 'benchmark_mpi.c' 'benchmark_mpi.h' 'Makefile' "Data/big_input_$SIZE.csv")
THREAD_NR=(1 2 4 8 16 40)
MACHINE_NR=(1 2 4)

if [ "$#" -ne 1 ]; then
    echo 'Error, usage: run_comparison_exp.sh (all|plot)'
    exit
fi

if [ "$OPTION" = "all" ]; then
    # Generate input file
    python3 ./Script/generate_dataset.py 1000
    
    # Copy files on all machines & compile 


    #  Run the experiments


fi


# Plot the results found


exit

