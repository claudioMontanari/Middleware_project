# Middleware Technologies for Distributed Systems 


## OpenMP/MPI Project

This repository provides an efficient implementation of the well known __[k-means](https://en.wikipedia.org/wiki/K-means_clustering)__ clustering algorithm, developed during the Fall 2019 Semester at Politecnico di Milano for the class of Middleware Technologies for Distributed Systems. 


## Project Design 

The chosen architecture is meant to perform better in a scenario of heavy workload. In particular, the execution flow works as follow:

1. The input dataset is split evenly among the machines available and loaded into each machine memory
2. The __master__ node takes care of initializing the array of __centroids__ (pick some random points from the dataset) and some other variables used as accumulators (centroids coordinates and centroids weight)
3. The __centroids__ are broadcasted by the master to all the machines available (included the master)
4. Each machine now can start assigning a centroid to each of the points taken from its dataset
5. This task is parallelized using the __OpenMP__ `#pragma parallel` directive 
6. Once this phase is finished the __master__ node gathers the accumulators for the centroids coordinates and weight
7. Finally, the __master__ updates the value of the centroids and check if they moved; if so, the procedure is repeated, otherwise we stop, save to a file the centroids and cleanup memory     

The picture below should help to better understand and visualize the implemented architecture. 

![test](./kmeans_MPI_OMP.png)

## Getting Started 

In order to have the project up and running on your system follow these instructions:


### Prerequisites and Installing

The project requires __ssh__ and __parallel-ssh__ installed and properly configured. Other tools used for running some tests (like pip3, numpy and openmpi ) will be installed on the cluster if you execute the __init_env.sh__ script. In particular, the network configuration is supposed to have four nodes called respectively *node-0, node-1, node-2, node-3*, you can change such names in the above mentioned script. 


## Running the tests

After installation the algorithm can be tested in different ways:

- You can just run `make` and the proper executable will be compiled under the name of __kmeans__ 
- You can run the __run_experiment.sh__ script giving as input:
	- the path to the .csv file of the data-points
	- the path to the output .csv file
	- the number of centroids 
	- the number of dimensions 
	- optionally you can specify if you want to have a 2D plot of the resulting clustering    
The command will automatically synchronize a common directory between the cluster's machines, compile the program and run the test. 

## Results 

Here are some plots of the results found. On the x-axis we have the size of the dataset while on the y-axis the execution time. Different lines represents different MPI/OpenMP configurations; in particular, .... 