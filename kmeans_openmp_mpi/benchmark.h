#ifndef BENCHMARK_H_INCLUDED
#define BENCHMARK_H_INCLUDED

#pragma once

#include "datapoint.h"

//long SIZE = 0;
int NR_THREADS;
int NR_CENTROIDS;
int NR_DIMENSIONS;

typedef struct pthread_data {
	unsigned long iterations;
	Datapoint* dataset;
	Centroid* centroids;
	int nr_dimensions;
	int nr_centroids;
	int size;
	char * out_file;
	volatile int stop;
} pthread_data_t;

#endif // BENCHMARK_H_INCLUDED