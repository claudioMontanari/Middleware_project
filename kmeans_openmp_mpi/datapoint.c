#include "datapoint.h"

void print_point(Datapoint* p){
	
	int i;

	for(i = 0; i < p->dimensions - 1; i++){
		printf("%lf, ", p->coordinates[i]);
	}
	printf("%lf\n", p->coordinates[i]);
}

void print_centroid(Centroid* c){
	
	int i;

	for(i = 0; i < c->dimensions - 1; i++){
		printf("%lf, ", c->coordinates[i]);
	}
	printf("%lf\n", c->coordinates[i]);
}


double distance(Datapoint* p, Centroid* c){

	double distance = 0.0, differential = 0.0;
	int i;

	for ( i = 0; i < p->dimensions; i++){
		differential = p->coordinates[i] - c->coordinates[i];
		distance+= differential*differential;
	}

	return sqrt(distance);
}

