import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

mu_list = [[5, 5], [5, 10], [10, 10], [10, 5]]
sigma = [[1, 0], [0, 1]]
nr_points_per_centroid = 5000
nr_points = nr_points_per_centroid * len(mu_list)

centroids = [[nr_points, nr_points]]

for mu in mu_list:
    #print(*mu, sep =", ")
    n = np.random.multivariate_normal(mu, sigma, nr_points_per_centroid)
    #print(*n, sep=", ")
    centroids = np.append(centroids, n, axis=0)
    #print(*centroids, sep=", ")


plt.scatter(centroids[1:, 0], centroids[1:, 1])
plt.show(block=True)
np.savetxt("output.csv", centroids[1:], delimiter=", ", fmt="%.8f", header=str(nr_points))

