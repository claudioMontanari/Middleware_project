# Middleware Technologies for Distributed Systems 


## OpenMP/MPI Project

This repository provides an efficient implementation of the well known __[k-means](https://en.wikipedia.org/wiki/K-means_clustering)__ clustering algorithm, developed during the Fall 2019 Semester at Politecnico di Milano for the class of Middleware Technologies for Distributed Systems. 


## Project Design 

The chosen architecture is meant to perform better in a scenario of heavy workload. In particular, the execution flow works as follow:

1. The input dataset is split evenly among the machines available and loaded into each machine memory
2. The __master__ node takes care of initializing the array of __centroids__ (pick some random points from the dataset) and some other variables used as accumulators (centroids coordinates and centroids weight)
3. The __centroids__ are broadcasted by the master to all the machines available
4. Each machine now can start assigning a centroid to each of the points taken from its dataset
5. This task is parallelized using the __OpenMP__ `#pragma parallel` directive 
6. Once this phase is finished the __master__ node gathers the accumulators for the centroids coordinates and weight
7. Finally, the __master__ updates the value of the centroids and check if they moved; if so, the procedure is repeated, otherwise we stop, save to a file the centroids and cleanup memory     

The picture below should help to better understand and visualize the implemented architecture.

![test](./kmeans_MPI_OMP.png)

## Getting Started 



### Prerequisites



### Installing



## Running the tests



