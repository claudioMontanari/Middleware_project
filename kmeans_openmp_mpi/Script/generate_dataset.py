import numpy as np
import matplotlib.pyplot as plt
import sys

# Usage: python3 generate_dataset.py nr_points_per_centroid output_filename 
if (len(sys.argv) <= 1 or len(sys.argv) > 3):
    print("Error, usage: python3 generate_dataset.py nr_points_per_centroid output_filename")
    sys.exit()

mu_list = [[50, 50], [50, 100], [100, 100], [100, 50], [100, 200], [200, 100]]
sigma = [[50, 0], [0, 50]]

mu_list = [[50, 50, 50], [50, 100, 100], [100, 100, 100], [100, 50, 50], [100, 200, 200], [200, 100, 100]]
sigma = [[50, 0, 0], [0, 50, 0], [0, 0, 50]]

nr_points_per_centroid = int(sys.argv[1])
output_file = sys.argv[2] if (len(sys.argv) == 3) else ''

nr_centroids = len(mu_list)
nr_points = nr_points_per_centroid * nr_centroids

#centroids = [[nr_points, nr_points]]
centroids = [[nr_points, nr_points, nr_points]]

for mu in mu_list:
    #print(*mu, sep =", ")
    n = np.random.multivariate_normal(mu, sigma, nr_points_per_centroid)
    #print(*n, sep=", ")
    centroids = np.append(centroids, n, axis=0)
    #print(*centroids, sep=", ")


#plt.scatter(centroids[1:, 0], centroids[1:, 1])
#plt.show(block=True)
np.random.shuffle(centroids[1:])
if (len(output_file) == 0 ):
    output_file = "../Data/input_dim_2_k_"+str(nr_centroids)+"_"+str(nr_points)+"pts.csv"
np.savetxt(output_file, centroids[1:], delimiter=", ", fmt="%.8f", header=str(nr_points))
print("data saved at: " + output_file)
