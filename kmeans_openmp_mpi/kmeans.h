#ifndef KMEANS_H_INCLUDED
#define KMEANS_H_INCLUDED

#include "datapoint.h"

long k_means(Datapoint* dataset, Centroid* centroids, int dimensions, int nr_centroids, long size, volatile int* stop);

#endif // KMEANS_H_INCLUDED