import numpy as np
import matplotlib.pyplot as plt
import sys


mu_list = [[50, 50], [50, 100], [100, 100], [100, 50], [100, 200], [200, 100]]
sigma = [[50, 0], [0, 50]]
nr_points_per_centroid = sys.argv[0]
nr_centroids = len(mu_list)
nr_points = nr_points_per_centroid * nr_centroids
centroids = [[nr_points, nr_points]]

for mu in mu_list:
    #print(*mu, sep =", ")
    n = np.random.multivariate_normal(mu, sigma, nr_points_per_centroid)
    #print(*n, sep=", ")
    centroids = np.append(centroids, n, axis=0)
    #print(*centroids, sep=", ")


#plt.scatter(centroids[1:, 0], centroids[1:, 1])
#plt.show(block=True)
np.random.shuffle(centroids[1:])
output_file = "../Data/input_dim_2_k_"+str(nr_centroids)+"_"+str(nr_points)+"pts.csv"
np.savetxt(output_file, centroids[1:], delimiter=", ", fmt="%.8f", header=str(nr_points))
print("data saved at: " + output_file)
