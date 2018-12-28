#ifndef DATAPOINT_H_INCLUDED
#define DATAPOINT_H_INCLUDED

#include <stdio.h>
#include <math.h>

typedef struct {
    double* coordinates;
    int dimensions;
    int centroid;
} Datapoint;

typedef struct {
    double* coordinates;
    int dimensions;
    double* accumulators;
    long num_points;
} Centroid;

double distance(Datapoint* p, Centroid* c);

void print_point(Datapoint* p);

void print_centroid(Centroid* c);

#endif // DATAPOINT_H_INCLUDED