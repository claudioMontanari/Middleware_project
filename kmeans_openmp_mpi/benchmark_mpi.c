#include <getopt.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <mpi.h>
#include <assert.h>

#include "benchmark.h"
#include "kmeans.h"
#include "datapoint.h"

#define DEFAULT_DURATION 	10000
#define DEFAULT_ITERATIONS	10
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
	printf("  -t: experiment running time in milliseconds (default %d)\n", DEFAULT_DURATION);
	printf("  -n: number of threads per  (default %d)\n", DEFAULT_NTHREADS);
	printf("  -c: number of centroids (default %d)\n", DEFAULT_CENTROIDS);
	printf("  -d: number of dimensions (default %d)\n", DEFAULT_DIMENSIONS);
	printf("  -p: path to input .csv file (default path %s)\n", DEFAULT_INPUT);
	printf("  -o: path to output .csv file (default path %s)\n", DEFAULT_OUTPUT);
	printf("  -r: number of restarts (default %d)\n", DEFAULT_RESTARTS);
}

// Creates an array of random floats. Each number has a value from 0 - 1
float* create_rand_nums(const int num_elements) {
  float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
  assert(rand_nums != NULL);
  for (int i = 0; i < num_elements; i++) {
    rand_nums[i] = (rand() / (float)RAND_MAX);
  }
  return rand_nums;
}

// Distance**2 between d-vectors pointed to by v1, v2.
float distance2(const float *v1, const float *v2, const int d) {
  float dist = 0.0;
  for (int i=0; i<d; i++) {
    float diff = v1[i] - v2[i];
    dist += diff * diff;
  }
  return dist;
}

// Assign a site to the correct cluster by computing its distances to
// each cluster centroid.
int assign_site(const float* site, float* centroids,
		const int k, const int d) {
  int best_cluster = 0;
  float best_dist = distance2(site, centroids, d);
  float* centroid = centroids + d;
  for (int c = 1; c < k; c++, centroid += d) {
    float dist = distance2(site, centroid, d);
    if (dist < best_dist) {
      best_cluster = c;
      best_dist = dist;
    }
  }
  return best_cluster;
}


// Add a site (vector) into a sum of sites (vector).
void add_site(const float * site, float * sum, const int d) {
  for (int i=0; i<d; i++) {
    sum[i] += site[i];
  }
}

// Print the centroids one per line.
void print_centroids(float * centroids, const int k, const int d) {
  float *p = centroids;
  printf("Centroids:\n");
  for (int i = 0; i<k; i++) {
    for (int j = 0; j<d; j++, p++) {
      printf("%f ", *p);
    }
    printf("\n");
  }
}

int main(int argc, char** argv) {
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
	printf("Test time:		%d\n", duration);
	printf("Thread number:		%d\n", nr_threads);
	printf("Centroids number:	%d\n", nr_centroids);
	printf("Number of dimensions:	%d\n", nr_dimensions);
	printf("Restarts number:	%d\n", nr_restarts);
	printf("Input file:		%s\n", input_file);

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


  if (argc != 4) {
    fprintf(stderr,
	    "Usage: kmeans num_sites_per_proc num_means num_dimensions\n");
    exit(1);
  }

  // Get stuff from command line:
    // number of sites per processor.
    // number of processors comes from mpirun command line.  -n
  int sites_per_proc = atoi(argv[1]);
  int k = atoi(argv[2]);  // number of clusters.
  int d = atoi(argv[3]);  // dimension of data.
  // Seed the random number generator to get different results each time
  //  srand(time(NULL));
  // No, we'd like the same results.
  srand(31359);

  // Initial MPI and find process rank and number of processes.
  MPI_Init(NULL, NULL);
  int rank, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  //
  // Data structures in all processes.
  //
  // The sites assigned to this process.
  float* sites;  
  assert(sites = malloc(sites_per_proc * d * sizeof(float)));
  // The sum of sites assigned to each cluster by this process.
  // k vectors of d elements.
  float* sums;
  assert(sums = malloc(k * d * sizeof(float)));
  // The number of sites assigned to each cluster by this process. k integers.
  int* counts;
  assert(counts = malloc(k * sizeof(int)));
  // The current centroids against which sites are being compared.
  // These are shipped to the process by the root process.
  float* centroids;
  assert(centroids = malloc(k * d * sizeof(float)));
  // The cluster assignments for each site.
  int* labels;
  assert(labels = malloc(sites_per_proc * sizeof(int)));
  
  //
  // Data structures maintained only in root process.
  //
  // All the sites for all the processes.
  // site_per_proc * nprocs vectors of d floats.
  float* all_sites = NULL;
  // Sum of sites assigned to each cluster by all processes.
  float* grand_sums = NULL;
  // Number of sites assigned to each cluster by all processes.
  int* grand_counts = NULL;
  // Result of program: a cluster label for each site.
  int* all_labels;
  if (rank == 0) {
    all_sites = create_rand_nums(d * sites_per_proc * nprocs);
    // Take the first k sites as the initial cluster centroids.
    for (int i = 0; i < k * d; i++) {
      centroids[i] = all_sites[i]; 
    }
    print_centroids(centroids, k, d);
    assert(grand_sums = malloc(k * d * sizeof(float)));
    assert(grand_counts = malloc(k * sizeof(int)));
    assert(all_labels = malloc(nprocs * sites_per_proc * sizeof(int)));
  }

  // Root sends each process its share of sites.
  MPI_Scatter(all_sites,d*sites_per_proc, MPI_FLOAT, sites,
              d*sites_per_proc, MPI_FLOAT, 0, MPI_COMM_WORLD);

  
  float norm = 1.0;  // Will tell us if centroids have moved.
  
  while (norm > 0.00001) { // While they've moved...

    // Broadcast the current cluster centroids to all processes.
    MPI_Bcast(centroids, k*d, MPI_FLOAT,0, MPI_COMM_WORLD);

    // Each process reinitializes its cluster accumulators.
    for (int i = 0; i < k*d; i++) sums[i] = 0.0;
    for (int i = 0; i < k; i++) counts[i] = 0;

    // Find the closest centroid to each site and assign to cluster.
    float* site = sites;
    for (int i = 0; i < sites_per_proc; i++, site += d) {
      int cluster = assign_site(site, centroids, k, d);
      // Record the assignment of the site to the cluster.
      counts[cluster]++;
      add_site(site, &sums[cluster*d], d);
    }

    // Gather and sum at root all cluster sums for individual processes.
    MPI_Reduce(sums, grand_sums, k * d, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(counts, grand_counts, k, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
      // Root process computes new centroids by dividing sums per cluster
      // by count per cluster.
      for (int i = 0; i<k; i++) {
	for (int j = 0; j<d; j++) {
	  int dij = d*i + j;
	  grand_sums[dij] /= grand_counts[i];
	}
      }
      // Have the centroids changed much?
      norm = distance2(grand_sums, centroids, d*k);
      printf("norm: %f\n",norm);
      // Copy new centroids from grand_sums into centroids.
      for (int i=0; i<k*d; i++) {
	centroids[i] = grand_sums[i];
      }
      print_centroids(centroids,k,d);
    }
    // Broadcast the norm.  All processes will use this in the loop test.
    MPI_Bcast(&norm, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
  }

  // Now centroids are fixed, so compute a final label for each site.
  float* site = sites;
  for (int i = 0; i < sites_per_proc; i++, site += d) {
    labels[i] = assign_site(site, centroids, k, d);
  }

  // Gather all labels into root process.
  MPI_Gather(labels, sites_per_proc, MPI_INT,
	     all_labels, sites_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

  // Root can print out all sites and labels.
  if ((rank == 0) && 1) {
    float* site = all_sites; 
    for (int i = 0;
	 i < nprocs * sites_per_proc;
	 i++, site += d) {
      for (int j = 0; j < d; j++) printf("%f ", site[j]);
      printf("%4d\n", all_labels[i]);
    }
  }
      
  MPI_Finalize();

}
