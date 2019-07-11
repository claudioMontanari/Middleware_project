#include "kmeans.h"
#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

static const double TOLERANCE = 0.00000000001;

// Centroids are initialized using randomly chosen points in the dataset given
void init_centroids(Datapoint* dataset, Centroid* centroids, int dimensions, int nr_centroids, long size){

	int i, j, rand_position;
	time_t t;

	srand((unsigned) time(&t));
	for (i = 0; i < nr_centroids; i++){
		
		rand_position = rand() % size;
		centroids[i].num_points = 0;
		for (j = 0; j < dimensions; j++){
			centroids[i].coordinates[j] = dataset[rand_position].coordinates[j];	
			centroids[i].accumulators[j] = 0.0;
		}

	}
}

void compute_distance(Datapoint* dataset, Centroid* centroids, int dimensions, int nr_centroids, long size){

	int i, j, min_centroid;
	double min_distance, temp_distance;

	
	#pragma omp parallel for num_threads(NR_THREADS) default(none) \
	shared(dataset, centroids, dimensions, nr_centroids, size) private(i, j, min_centroid, min_distance, temp_distance)
	for ( i = 0; i < size; ++i){
		
		min_centroid = 0;
		min_distance = distance(&dataset[i], &centroids[0]);

		for( j = 1; j < nr_centroids; j++){
			temp_distance = distance(&dataset[i], &centroids[j]);

			if( temp_distance < min_distance ){
				min_distance = temp_distance;
				min_centroid = j;
			}
		}
		//TODO: what if here I do something like: 
		//	temp_centroids[min_centroid]++;
		//where temp_centroids is a private array of int that keeps track of the number of points for a given centroid.
		//At the end I'll need to aggregate only this partial results. 
		//(It should work better if the datapoints don't fit in the cache).
		
		dataset[i].centroid = min_centroid;
	}


}

// TODO: Check if doing this without parallelization increases performance (overall we have few dimensions/centroids)
void update_accumulators(Datapoint* dataset, Centroid* centroids, int dimensions, int nr_centroids, long size){

	int i, k;
	long j;

	#pragma omp parallel for num_threads(NR_THREADS) default(none) \
	shared(dataset, centroids, dimensions, nr_centroids, size) private(i, j, k)
	for ( i = 0; i < nr_centroids; i++){
		
		for ( j = 0; j < size; j++){
			
			if( dataset[j].centroid == i){
				centroids[i].num_points++;
				for ( k = 0; k < dimensions; k++){
					centroids[i].accumulators[k] += dataset[j].coordinates[k];	
				}
			}
		
		}
	}
	
}

int update_centroids(Centroid* centroids, int nr_centroids, int dimensions){

	int i, j, changed = 0;
	double new_coordinate;

	//TODO: Compare performance inverting the loop

	for ( i = 0; i < nr_centroids; i++){
			
			for ( j = 0; j < dimensions; j++){

				new_coordinate = centroids[i].accumulators[j] / centroids[i].num_points;
				
				if( new_coordinate - centroids[i].coordinates[j] >= TOLERANCE )
					changed = 1;
				
				centroids[i].coordinates[j] = new_coordinate;
				centroids[i].accumulators[j] = 0.0;
			}
			centroids[i].num_points = 0;

	}

	return changed;
}

int has_changed(Centroid* old_centroids, Centroid* new_centroids, int nr_centroids, int dimensions){

	int i, j, changed = 0;
	
	for( i = 0; i < nr_centroids; i++){
		for ( j = 0; j < dimensions; j++){
			if(new_centroids[i].coordinates[j] - old_centroids[i].coordinates[j] >= TOLERANCE)
				changed = 1;
		}
	}

	return changed;
}

long k_means(Datapoint* dataset, Centroid* centroids, int dimensions, int nr_centroids, long size, volatile int* stop){

	/*
	* TODO: 
	*  1. Initialize centroids - single thread
	*  2. Compute distance between datapoints and centroids - multi thread
	*  3. Update Centroids accumulators - multi threaded
	*  4. Update Centroids position and reset accumulators
	*  5. Check exit condition: time expired or same centroids- in case repeat from 2.  
	*/

	long iterations = 0;
	int i, changed = 0;

	init_centroids(dataset, centroids, dimensions, nr_centroids, size);

	do{
		/*
		printf("iteration: %ld , stop: %d, changed: %d, tolerance %f \n", iterations, *stop, changed, TOLERANCE);
		printf("centroids: \n");
		for( i = 0; i < nr_centroids; i++){
			printf("%d: ", i);
			print_centroid(&centroids[i]);
		}
		*/
		
		iterations++;
		compute_distance(dataset, centroids, dimensions, nr_centroids, size);
		update_accumulators(dataset, centroids, dimensions, nr_centroids, size);
		changed = update_centroids(centroids, nr_centroids, dimensions);
		
	} while( *stop != 1 && changed != 0);
	
	return iterations;
}
