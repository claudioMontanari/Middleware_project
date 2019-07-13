#include <getopt.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include "benchmark.h"
#include "kmeans.h"
#include "datapoint.h"

#define DEFAULT_DURATION 	10000
#define DEFAULT_NTHREADS 	4
#define DEFAULT_CENTROIDS   	3
#define DEFAULT_DIMENSIONS  	2
#define DEFAULT_INPUT   	"./Data/input.csv"
#define DEFAULT_OUTPUT 		"./Data/output.csv"
#define DEFAULT_RESTARTS   	0

void print_args(void)
{
	printf("Kmeans benchmark:\n");
	printf("options:\n");
	printf("  -h: print help message\n");
	printf("  -t: test running time in milliseconds (default %d)\n", DEFAULT_DURATION);
	printf("  -n: number of threads (default %d)\n", DEFAULT_NTHREADS);
	printf("  -c: number of centroids (default %d)\n", DEFAULT_CENTROIDS);
	printf("  -d: number of dimensions (default %d)\n", DEFAULT_DIMENSIONS);
	printf("  -p: path to input .csv file (default path %s)\n", DEFAULT_INPUT);
	printf("  -o: path to output .csv file (default path %s)\n", DEFAULT_OUTPUT);
	printf("  -r: number of restarts (default %d)\n", DEFAULT_RESTARTS);
}


long build_data_points(FILE* fp, Datapoint** dataset_ptr, int dimensions){
	
	long size = 0;


	if ( !fscanf(fp, "%ld", &size) ){
		printf("Error while parsing the input file\n");
		return -1;
	}
	if( size <= 0){
		printf("Size value must be a positive integer! \n");
		return -1;
	}

	*dataset_ptr = (Datapoint*) malloc(sizeof(Datapoint) * size);
	Datapoint* dataset = *dataset_ptr;
	if( dataset == NULL){
		printf("Memory allocation error\n");
		return -1;
	}

	long index = 0;

	while( !feof(fp) && (index < size) ){

		dataset[index].coordinates = (double *) malloc(sizeof(double) * dimensions);
	
		dataset[index].centroid = 0;
		dataset[index].dimensions = dimensions;			

		int i;
		for(i = 0; i < dimensions - 1; i++){
			if( !fscanf(fp, "%lf, ", &dataset[index].coordinates[i]) )
				return -1;
		}
		if( !fscanf(fp, "%lf\n", &dataset[index].coordinates[i]) ) 
			return -1;

		//print_point(&dataset[index]);

		index++;
	}

	return size;
}

void build_centroids(Centroid** centroids_ptr, int size, int dimensions){

	int i;

	*centroids_ptr = (Centroid* ) malloc(sizeof(Centroid) * size);
	Centroid* centroids = *centroids_ptr;
	for(i = 0; i < size; i++){
		centroids[i].coordinates = (double* ) malloc(sizeof(double) * dimensions);
		centroids[i].accumulators = (double* ) malloc(sizeof(double) * dimensions);
		centroids[i].dimensions = dimensions;
		centroids[i].num_points = 0;
	}

}

static void *bench_thread(void *data) {

	pthread_data_t *d = (pthread_data_t *)data;

	d->iterations = k_means(d->dataset, d->centroids, d->nr_dimensions, d->nr_centroids, d->size, &d->stop);

	return NULL;
}

void save_to_file(FILE* f_out, Centroid* centroids, int nr_centroids, int dimensions){

	int i, j;

	for (i = 0; i < nr_centroids; i++){

		for ( j = 0; j < dimensions - 1; j++){
			 fprintf(f_out, "%lf, ", centroids[i].coordinates[j]); 			
		}

		fprintf(f_out, "%lf\n", centroids[i].coordinates[dimensions - 1]); 
	}

}

int main(int argc, char *argv[])
{
	struct option bench_options[] = {
		{"help",           		no_argument,       NULL, 'h'},
		{"time-duration",  		required_argument, NULL, 't'},
		{"num-of-threads", 		required_argument, NULL, 'n'},
		{"num-of-centroids",		required_argument, NULL, 'c'},
		{"num-of-dimensions",		required_argument, NULL, 'd'},
		{"input-file",     		required_argument, NULL, 'p'},
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
	
	FILE* fp, *f_out;
	Datapoint* dataset = NULL;
	Centroid* centroids = NULL;

	pthread_t thread;
	pthread_data_t data;
	struct timeval start, end;
	struct timespec timeout;

	while (1) {
		c = getopt_long(argc, argv, "ht:n:c:d:p:o:r:", bench_options, &i);

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
		case 'p':
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

	printf("Kmeans benchmark\n");
	printf("Test time:     %d\n", duration);
	printf("Thread number: %d\n", nr_threads);
	printf("Centroids number: %d\n", nr_centroids);
	printf("Number of dimensions:  %d\n", nr_dimensions);
	printf("Restarts number:   %d\n", nr_restarts);
	printf("Input file:   %s\n", input_file);

	/*
	* TODO:
	*  1. parse the input file and build the data structure (close the input file)
	*  2. start the timing 
	*  3. start k-means
	*  4. stop timing 
	*  5. write results in output file (append mode, and then close it)
	*  6. check nr_restarts and in case repeat from 2.
	*  7. print some statistics  
	*/

	size = build_data_points(fp, &dataset, nr_dimensions);
	if( size < 0 ) 
		goto out;
	fclose(fp);

	build_centroids(&centroids, nr_centroids, nr_dimensions);

	//SIZE = size;
	NR_CENTROIDS = nr_centroids;
	NR_DIMENSIONS = nr_dimensions;
	NR_THREADS = nr_threads;

	data.iterations = iterations;
	data.dataset = dataset;
	data.centroids = centroids;
	data.nr_dimensions = nr_dimensions;
	data.nr_centroids = nr_centroids;
	data.size = size; 
	data.stop = 0;

	timeout.tv_sec = duration / 1000;
	timeout.tv_nsec = (duration % 1000) * 1000000;

	
	for( j = 0; j < nr_restarts + 1; j++){
	
		if ( pthread_create(&thread, NULL, bench_thread, (void *)(&data)) != 0) {
			printf("failed to create thread for iteration %d\n", j);
			goto out;
		}


		//nanosleep(&timeout, NULL);
		//data.stop = 1;

		if ( pthread_join(thread, NULL) != 0) {
			printf("failed to join thread for iteration %d\n", j);
			goto out;
		}		

		gettimeofday(&end, NULL);

		duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);
		printf("duration: %d ms\n", duration);		
		printf("iterations: %ld\n", data.iterations);
		printf("centroids: \n");
		for( i = 0; i < nr_centroids; i++){
			printf("%d: ", i);
			print_centroid(&centroids[i]);
		}

		save_to_file( f_out, centroids, nr_centroids, nr_dimensions);

	}

	//Clean up streams and memory
	fclose(f_out);

	for( j = 0; j < size; j++){
		free(dataset[j].coordinates);
	}
	free(dataset);
	for( j = 0; j < nr_centroids; j++){
		free(centroids[j].coordinates);
		free(centroids[j].accumulators);
	}
	free(centroids);

out: 
	return 0;

}
