#include <getopt.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <mpi.h>
#include <assert.h>

#include "benchmark_mpi.h"

#define DEFAULT_DURATION 	10000
#define DEFAULT_ITERATIONS	10
#define DEFAULT_NTHREADS 	4
#define DEFAULT_CENTROIDS   	3
#define DEFAULT_DIMENSIONS  	3
#define DEFAULT_INPUT   	"./Data/input.csv"
#define DEFAULT_OUTPUT 		"./Data/output.csv"
#define DEFAULT_RESTARTS   	0
#define MIN_NORM		0.000000001

void print_args(void)
{
	printf("Kmeans benchmark:\n");
	printf("options:\n");
	printf("  -h: print help message\n");
	printf("  -t: experiment running time in milliseconds (default %d)\n", DEFAULT_DURATION);
	printf("  -n: number of threads per machine (default %d)\n", DEFAULT_NTHREADS);
	printf("  -c: number of centroids (default %d)\n", DEFAULT_CENTROIDS);
	printf("  -d: number of dimensions (default %d)\n", DEFAULT_DIMENSIONS);
	printf("  -i: path to input .csv file (default path %s)\n", DEFAULT_INPUT);
	printf("  -o: path to output .csv file (default path %s)\n", DEFAULT_OUTPUT);
	printf("  -r: number of restarts (default %d)\n", DEFAULT_RESTARTS);
}

void print_point(double* point, int dimensions){
  	int i;
  	for(i = 0; i < dimensions - 1; i++){
		printf("%lf ", point[i]);
	}
	printf("%lf\n", point[i]);
}

long build_data_points(FILE* fp, double** dataset_ptr, const int dimensions, const int rank, const int group_size){
	
	long size = 0, points_per_machine;


	if ( !fscanf(fp, "%ld\n", &size) ){
		printf("Error while parsing the input file\n");
		return -1;
	}
	if( size <= 0){
		printf("Size value must be a positive integer! \n");
		return -1;
	}
	points_per_machine = size / group_size;
	  
	*dataset_ptr = (double*) malloc(sizeof(double) * points_per_machine * dimensions);

	double* dataset = *dataset_ptr;
	if( dataset == NULL){
		printf("Memory allocation error\n");
		return -1;
	}

	long index = 0;
	long offset = points_per_machine * rank;			//TODO: this works as long as the rank starts form 0, otherwise needs (rank-1)
	char buffer[80];
	for(int i = 0; i < offset; i++)
		fgets(buffer, 80, fp);
	
	while( !feof(fp) && index < (points_per_machine * dimensions) ){

		int i;
		for(i = 0; i < dimensions - 1; i++){
			if( !fscanf(fp, "%lf, ", &dataset[index + i]))
				return -1;
		}
		if( !fscanf(fp, "%lf\n", &dataset[index + dimensions - 1]) ) 
			return -1;
#ifdef DEBUG
		printf("[%d] - ", rank);
		print_point(&dataset[index], dimensions);
#endif
		index+=dimensions;
	}
	return size;
}

int allocate_centroids(double** centroids_ptr, int nr_centroids, int nr_dimensions){
  	*centroids_ptr = (double*)(malloc(sizeof(double) * nr_centroids * nr_dimensions));
	if(centroids_ptr == NULL){
	  	printf("Memory allocation error!\n");
	  	return -1;
	}
	double * centroids = *centroids_ptr;
	for (int i = 0; i < nr_centroids * nr_dimensions; i++)
	 	centroids[i] = 0;
}

int init_centroids(double* centroids, int k, int dimensions, double* dataset, long size, int rank){

  	int i, j;
	srand(time(NULL));
	for(i = 0; i < k*dimensions; i+=dimensions){
	  	j = (rand() % size) * dimensions;
	  	for (int p = 0; p < dimensions; p++)
		  	centroids[i + p] = dataset[j + p];
#ifdef DEBUG
		printf("[%d] - picked: %d\n", rank, j / dimensions);
		print_point(&centroids[i], dimensions);
#endif
	}
}


void save_to_file(FILE* f_out, double* centroids, int nr_centroids, int dimensions){

	int i, j;

	for (i = 0; i < nr_centroids; i++){

		for ( j = 0; j < dimensions - 1; j++){
			 fprintf(f_out, "%lf, ", centroids[i*dimensions + j]); 			
		}

		fprintf(f_out, "%lf\n", centroids[i*dimensions + dimensions - 1]);
	}

}


int main(int argc, char** argv) {
	struct option bench_options[] = {
		{"help",           		no_argument,       NULL, 'h'},
		{"time-duration",  		required_argument, NULL, 't'},
		{"num-of-threads", 		required_argument, NULL, 'n'},
		{"num-of-centroids",		required_argument, NULL, 'c'},
		{"num-of-dimensions",		required_argument, NULL, 'd'},
		{"input-file",     		required_argument, NULL, 'i'},
		{"output-file",    		required_argument, NULL, 'o'},
		{"restarts",       		required_argument, NULL, 'r'},
		{0,                		0,                 0,    0  }
	};


	int c, i, j;
	long size = 0, iterations = 0;
	int duration = 					DEFAULT_DURATION;
	int nr_threads = 				DEFAULT_NTHREADS;
	int nr_centroids = 				DEFAULT_CENTROIDS;
	int nr_dimensions =			 	DEFAULT_DIMENSIONS;
	char* input_file = 				DEFAULT_INPUT;
	char* output_file = 				DEFAULT_OUTPUT;
	int nr_restarts = 				DEFAULT_RESTARTS;

	int nr_machines = 0;
	int rank = 0;
	
	FILE* fp, *f_out;
	double* dataset = NULL;
	double* old_centroids = NULL;
	double* new_centroids = NULL;
	int* points_per_centroid_accumulator = NULL;
	double* centroids_coordinates_accumulator = NULL;
	
	struct timeval start, end;
	struct timespec timeout;

	while (1) {
		c = getopt_long(argc, argv, "ht:n:c:d:i:o:r:", bench_options, &i);

		if (c == -1)
			break;

		if (c == 0 && bench_options[i].flag == 0)
			c = bench_options[i].val;

		switch(c) {
		case 'h':
			print_args();
			goto out;
		case 't':
			duration = atoi(optarg);
			break;
		case 'n':
			nr_threads = atoi(optarg);
			break;
		case 'c':
			nr_centroids = atoi(optarg);
			break;
		case 'd':
			nr_dimensions = atoi(optarg);
			break;
		case 'i':
			input_file = optarg;
			break;
		case 'o':
			output_file = optarg;
			break;
		case 'r':
			nr_restarts = atoi(optarg);
			break;
		default:
			printf("Error while processing options.\n");
			goto out;
		}
	}

	if (duration <= 0) {
		printf("invalid test time\n");
		goto out;
	}
	if (nr_threads <= 0) {
		printf("invalid thread number\n");
		goto out;
	}
	if (nr_centroids <= 0) {
		printf("invalid number of centroids\n");
		goto out;
	}
	if (nr_dimensions <= 0) {
		printf("invalid number of dimensions\n");
		goto out;
	}

	fp = fopen(input_file, "r");
	if ( fp == NULL ) {
		printf("unable to open the input file %s error number: %d\n", input_file, errno);
		goto out;
	}

	f_out = fopen(output_file, "w");
	if ( f_out == NULL ) {
		printf("unable to open the output file %s error number: %d\n", output_file, errno);
		goto out;
	}

	if (nr_restarts < 0 ) {
		printf("invalid number of restarts\n");
		goto out;
	}

	/*
	* TODO:
	*  1. initialize MPI environment
	*  2. parse the input file and build the data structure (close the input file)
	*  3. start the timing 
	*  4. start k-means
	*  5. stop timing 
	*  6. write results in output file (append mode, and then close it)
	*  7. check nr_restarts and in case repeat from 3.
	*  8. print some statistics
	*  9. cleanup memory  
	*/


	// Initialize MPI - find process rank and number of machines
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nr_machines);

	if (rank == 0){
		printf("Kmeans benchmark\n");
		printf("Test time:		%d\n", duration);
		printf("Machine number:		%d\n", nr_machines);
		printf("Thread number:		%d\n", nr_threads);
		printf("Centroids number:	%d\n", nr_centroids);
		printf("Number of dimensions:	%d\n", nr_dimensions);
		printf("Restarts number:	%d\n", nr_restarts);
		printf("Input file:		%s\n", input_file);
	}
	printf("rank: %d\n", rank);

	// Load the datapoints from a shared file - using rank as offset
#ifdef DEBUG
	printf("[%d] - building the datapoints\n", rank);
#endif
	size = build_data_points(fp, &dataset, nr_dimensions, rank, nr_machines);
	if(size < 0) 
		goto out;
	fclose(fp);

	allocate_centroids(&new_centroids, nr_centroids, nr_dimensions);
	allocate_centroids(&old_centroids, nr_centroids, nr_dimensions);

	// Let the root process initialize the centroids and send them to the others
	if (rank == 0){
#ifdef DEBUG
	  	printf("[%d] - building the centroids\n", rank);
#endif
	  	init_centroids(new_centroids, nr_centroids, nr_dimensions, dataset, size / nr_machines, rank);
	}

	char name[80];
	int name_len;
	MPI_Get_processor_name(name, &name_len);
	printf("[%d] - processor name %s\n", rank, name);

	if(rank == 0){
		gettimeofday(&start, NULL);
	}

	double norm = 1.0;
	while( norm > MIN_NORM){
		MPI_Bcast(new_centroids, nr_centroids*nr_dimensions, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		norm = 0;
	}
	
	if (rank == 0){
	  
		gettimeofday(&end, NULL);
		duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);
		printf("duration: %d ms\n", duration);		
		printf("iterations: %ld\n", iterations);
		printf("centroids: \n");
		
		for( i = 0; i < nr_centroids * nr_dimensions; i+=nr_dimensions){
			printf("%d: ", i / nr_dimensions);
			print_point(&new_centroids[i], nr_dimensions);
		}

		save_to_file(f_out, new_centroids, nr_centroids, nr_dimensions);
	}
	
out:
#ifdef DEBUG
	printf("[%d] - free memory\n", rank);
#endif
	if (rank == 0){
		free(old_centroids);
		free(new_centroids);
	}
	free(points_per_centroid_accumulator);
	free(centroids_coordinates_accumulator);
	free(dataset);
	MPI_Finalize();

	return 0;
}
